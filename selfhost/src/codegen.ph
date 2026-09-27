// =============================================================================
// codegen.ph - lowering of the checked AST to LLVM IR via the LLVM C API.
//
// Generic functions are monomorphized here: each (decl, type arguments)
// pair becomes one LLVM function, generated from the shared, already
// type-checked body by substituting the type arguments.
// =============================================================================
import "util.ph" as u;
import "diag.ph" as dg;
import "lexer.ph" as lx;
import "ast.ph" as ast;
import "types.ph" as ty;
import "sema.ph" as sema;
import "wasmops.ph" as wops;
import "llvm.ph" as L;

// What one codegen run produces (spec §17.7):
//   CG_WHOLE   the whole program in one object (-O2 and above by default)
//   CG_MODULE  one module's non-generic functions, plus the generic instances
//              it uses (link-once); everything else is referenced by name
//   CG_LINK    the per-program pieces: globals, error codes and names,
//              runtime state, test and host exports
pub const CG_WHOLE: i32 = 0;
pub const CG_MODULE: i32 = 1;
pub const CG_LINK: i32 = 2;

pub const EMIT_OBJ: i32 = 0;
pub const EMIT_LLVM: i32 = 1;
pub const EMIT_ASM: i32 = 2;

pub struct CgOptions {
    opt_level: i32,           // 0..3
    overflow_saturate: bool,  // --overflow=saturate (--release)
    cpu: []u8,                // nil = generic
    emit: i32,
    out_path: []u8,           // file to write
    stack_traces: bool,       // shadow stack for traces and profiles (--stack-traces=on, default)
    profile: bool,            // phalange profile: the shadow stack is sampled concurrently
    unit: i32,
    module: *mut ast.Module,  // CG_MODULE
}

// Names of exported functions (for the linker).
pub let mut g_export_names: u.List.[[]u8] = u.List.[[]u8].{ data: @ptr_from_int(*mut []u8, 0), len: 0, cap: 0 };
// Libraries named by extern "C" "lib" blocks (for the linker).
pub let mut g_link_libs: u.List.[[]u8] = u.List.[[]u8].{ data: @ptr_from_int(*mut []u8, 0), len: 0, cap: 0 };
// Build a test binary (phalange test): tests become roots and are exported.
pub let mut g_test_mode: bool = false;
// Set before codegen: the program spawns threads (wasm: shared memory and a
// thread entry export).
pub let mut g_threads: bool = false;
// Set by codegen: the output calls into runtime/native_host.c.
pub let mut g_needs_native_runtime: bool = false;

let mut O: *mut CgOptions = @ptr_from_int(*mut CgOptions, 0);
let mut ctx: *mut L.Context = @ptr_from_int(*mut L.Context, 0);
let mut M: *mut L.Module = @ptr_from_int(*mut L.Module, 0);
let mut B: *mut L.Builder = @ptr_from_int(*mut L.Builder, 0);
let mut TM: *mut L.TargetMachine = @ptr_from_int(*mut L.TargetMachine, 0);
let mut TD: *mut L.TargetData = @ptr_from_int(*mut L.TargetData, 0);
let mut T_i1: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_i8: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_i16: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_i32: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_i64: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_i128: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_isz: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_ptr: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_void: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_f32: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_f64: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut T_v128: *mut L.LType = @ptr_from_int(*mut L.LType, 0);

// ---------------------------------------------------------------------------
// Function instances
// ---------------------------------------------------------------------------
struct FnInst {
    decl: *mut ast.Decl,
    gargs: []mut *mut ast.Type,
    fnty: *mut ast.Type,  // concrete TY_FN
    fnv: *mut L.Value,
    llty: *mut L.LType,
    sret: bool,
    display: []u8,  // user-facing name, for stack traces and profiles
    next: *mut FnInst,
}

let mut inst_map: u.SymMap.[*mut FnInst] = u.SymMap.[*mut FnInst].{
    keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut *mut FnInst, 0), cap: 0, len: 0 };
let mut pending_head: *mut FnInst = @ptr_from_int(*mut FnInst, 0);
let mut pending_tail: *mut FnInst = @ptr_from_int(*mut FnInst, 0);

struct LoopCtx {
    brk: *mut L.BB,
    cont: *mut L.BB,
    defer_height: usize,
}

struct FnState {
    inst: *mut FnInst,
    decl: *mut ast.Decl,
    from: []mut *mut ast.Type,
    to: []mut *mut ast.Type,
    fnv: *mut L.Value,
    locals: []mut *mut L.Value,
    sret_ptr: *mut L.Value,
    ret: *mut ast.Type,  // concrete
    defers: u.List.[*mut ast.Stmt],
    loops: u.List.[LoopCtx],
    entry: *mut L.BB,
}

let mut F: *mut FnState = @ptr_from_int(*mut FnState, 0);

// ---------------------------------------------------------------------------
// Small wrappers (unnamed instructions)
// ---------------------------------------------------------------------------
fn vnil() *mut L.Value {
    return u.nil.[L.Value]();
}

fn tnil() *mut ast.Type {
    return u.nil.[ast.Type]();
}

fn bbnil() *mut L.BB {
    return u.nil.[L.BB]();
}

fn NN() *u8 {
    return "".ptr;
}

fn ci(t: *mut L.LType, v: u64) *mut L.Value {
    return L.LLVMConstInt(t, v, 0);
}

fn ci32(v: u64) *mut L.Value {
    return L.LLVMConstInt(T_i32, v, 0);
}

fn b_add(a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    return L.LLVMBuildAdd(B, a, b, NN());
}

fn b_sub(a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    return L.LLVMBuildSub(B, a, b, NN());
}

fn b_mul(a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    return L.LLVMBuildMul(B, a, b, NN());
}

fn b_and(a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    return L.LLVMBuildAnd(B, a, b, NN());
}

fn b_or(a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    return L.LLVMBuildOr(B, a, b, NN());
}

fn b_xor(a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    return L.LLVMBuildXor(B, a, b, NN());
}

fn b_not(a: *mut L.Value) *mut L.Value {
    return L.LLVMBuildNot(B, a, NN());
}

fn icmp(p: i32, a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    return L.LLVMBuildICmp(B, p, a, b, NN());
}

fn fcmp(p: i32, a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    return L.LLVMBuildFCmp(B, p, a, b, NN());
}

fn sel(c: *mut L.Value, a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    return L.LLVMBuildSelect(B, c, a, b, NN());
}

fn zext(v: *mut L.Value, t: *mut L.LType) *mut L.Value {
    return L.LLVMBuildZExt(B, v, t, NN());
}

fn sext(v: *mut L.Value, t: *mut L.LType) *mut L.Value {
    return L.LLVMBuildSExt(B, v, t, NN());
}

fn trunc(v: *mut L.Value, t: *mut L.LType) *mut L.Value {
    return L.LLVMBuildTrunc(B, v, t, NN());
}

fn bitcast(v: *mut L.Value, t: *mut L.LType) *mut L.Value {
    return L.LLVMBuildBitCast(B, v, t, NN());
}

fn ptrtoint(v: *mut L.Value, t: *mut L.LType) *mut L.Value {
    return L.LLVMBuildPtrToInt(B, v, t, NN());
}

fn inttoptr(v: *mut L.Value, t: *mut L.LType) *mut L.Value {
    return L.LLVMBuildIntToPtr(B, v, t, NN());
}

fn extract(v: *mut L.Value, i: u32) *mut L.Value {
    return L.LLVMBuildExtractValue(B, v, i, NN());
}

fn insert(agg: *mut L.Value, v: *mut L.Value, i: u32) *mut L.Value {
    return L.LLVMBuildInsertValue(B, agg, v, i, NN());
}

fn ld(t: *mut L.LType, p: *mut L.Value) *mut L.Value {
    return L.LLVMBuildLoad2(B, t, p, NN());
}

fn st(v: *mut L.Value, p: *mut L.Value) *mut L.Value {
    return L.LLVMBuildStore(B, v, p);
}

fn br(bb: *mut L.BB) void {
    _ = L.LLVMBuildBr(B, bb);
}

fn condbr(c: *mut L.Value, t: *mut L.BB, e: *mut L.BB) void {
    _ = L.LLVMBuildCondBr(B, c, t, e);
}

fn at_end(bb: *mut L.BB) void {
    L.LLVMPositionBuilderAtEnd(B, bb);
}

fn gep1(t: *mut L.LType, p: *mut L.Value, i: *mut L.Value) *mut L.Value {
    let mut ix: [1]*mut L.Value = [i];
    return L.LLVMBuildGEP2(B, t, p, &mut ix[0], 1, NN());
}

fn gep2(t: *mut L.LType, p: *mut L.Value, i: *mut L.Value, j: *mut L.Value) *mut L.Value {
    let mut ix: [2]*mut L.Value = [i, j];
    return L.LLVMBuildGEP2(B, t, p, &mut ix[0], 2, NN());
}

fn sgep(t: *mut L.LType, p: *mut L.Value, i: u32) *mut L.Value {
    return L.LLVMBuildStructGEP2(B, t, p, i, NN());
}

fn tint(bits: i32) *mut L.LType {
    return L.LLVMIntTypeInContext(ctx, @int_cast(u32, bits));
}

fn struct_ty(els: []mut *mut L.LType) *mut L.LType {
    return L.LLVMStructTypeInContext(ctx, &mut els[0], @int_cast(u32, els.len), 0);
}

fn const_struct(vals: []mut *mut L.Value) *mut L.Value {
    return L.LLVMConstStructInContext(ctx, &mut vals[0], @int_cast(u32, vals.len), 0);
}

fn fn_type(ret: *mut L.LType, ps: []mut *mut L.LType, vararg: bool) *mut L.LType {
    let mut va: i32 = 0;
    if vararg {
        va = 1;
    }
    if ps.len == 0 {
        return L.LLVMFunctionType(ret, u.nil.[*mut L.LType](), 0, va);
    }
    return L.LLVMFunctionType(ret, &mut ps[0], @int_cast(u32, ps.len), va);
}

fn call(fty: *mut L.LType, f: *mut L.Value, args: []mut *mut L.Value) *mut L.Value {
    if args.len == 0 {
        return L.LLVMBuildCall2(B, fty, f, u.nil.[*mut L.Value](), 0, NN());
    }
    return L.LLVMBuildCall2(B, fty, f, &mut args[0], @int_cast(u32, args.len), NN());
}

fn is_const_int(v: *mut L.Value) bool {
    return u.notnil(L.LLVMIsAConstantInt(v));
}

fn ct(t: *mut ast.Type) *mut ast.Type {
    if u.isnil(t) or u.isnil(F) or not t.has_params {
        return t;
    }
    return ty.subst(t, F.from, F.to);
}

fn internal_error(msg: []u8) void {
    u.eprint(u.f1("phalange: internal compiler error: %s\n", msg));
    u.quit(2);
}

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------
fn align_unit(a: i64) *mut L.LType {
    if a == 1 {
        return T_i8;
    }
    if a == 2 {
        return T_i16;
    }
    if a == 4 {
        return T_i32;
    }
    if a == 8 {
        return T_i64;
    }
    return T_v128;
}

fn lty(t: *mut ast.Type) *mut L.LType {
    if t.has_params {
        internal_error(u.f1("unsubstituted generic type %s", ty.type_str(t)));
    }
    if not u.isvnil(t.backend) {
        return @ptr_cast(*mut L.LType, t.backend);
    }
    let mut r = u.nil.[L.LType]();
    let k = t.kind;
    if k == ast.TY_VOID {
        r = T_void;
    } else if k == ast.TY_BOOL {
        r = T_i8;
    } else if k == ast.TY_INT {
        r = tint(t.bits);
    } else if k == ast.TY_FLOAT {
        r = T_f64;
        if t.bits == 32 {
            r = T_f32;
        }
    } else if k == ast.TY_VEC {
        let mut lane = tint(t.elem.bits);
        if t.elem.kind == ast.TY_FLOAT {
            lane = T_f64;
            if t.elem.bits == 32 {
                lane = T_f32;
            }
        }
        r = L.LLVMVectorType(lane, @int_cast(u32, t.lanes));
    } else if k == ast.TY_V128 {
        r = T_v128;
    } else if (k == ast.TY_PTR) or (k == ast.TY_FN) {
        r = T_ptr;
    } else if k == ast.TY_ERROR {
        r = T_i16;
    } else if k == ast.TY_SLICE {
        let mut el: [2]*mut L.LType = [T_ptr, T_isz];
        r = struct_ty(el[..]);
    } else if k == ast.TY_ARRAY {
        r = L.LLVMArrayType2(lty(t.elem), @bit_cast(u64, t.len));
    } else if k == ast.TY_ERRUNION {
        if t.elem.kind == ast.TY_VOID {
            let mut el: [1]*mut L.LType = [T_i16];
            r = struct_ty(el[..]);
        } else {
            let mut el: [2]*mut L.LType = [T_i16, lty(t.elem)];
            r = struct_ty(el[..]);
        }
    } else if k == ast.TY_STRUCT {
        r = L.LLVMStructCreateNamed(ctx, u.cz(ty.type_str(t)));
        t.backend = @ptr_cast(*mut void, r);
        let n = t.decl.fields.len;
        let el = u.newn.[*mut L.LType](n + 1);
        for i in 0..n {
            el[i] = lty(ty.type_field(t, i));
        }
        L.LLVMStructSetBody(r, &mut el[0], @int_cast(u32, n), 0);
    } else if k == ast.TY_UNION {
        r = L.LLVMStructCreateNamed(ctx, u.cz(ty.type_str(t)));
        t.backend = @ptr_cast(*mut void, r);
        let tag = lty(ty.union_tag_type(t));
        let psize = ty.union_payload_size(t);
        let palign = ty.union_payload_align(t);
        if psize == 0 {
            let mut el: [1]*mut L.LType = [tag];
            L.LLVMStructSetBody(r, &mut el[0], 1, 0);
        } else {
            let arr = L.LLVMArrayType2(align_unit(palign), @bit_cast(u64, ((psize + palign) - 1) / palign));
            let mut el: [2]*mut L.LType = [tag, arr];
            L.LLVMStructSetBody(r, &mut el[0], 2, 0);
        }
    } else {
        internal_error(u.f1("no LLVM type for %s", ty.type_str(t)));
    }
    t.backend = @ptr_cast(*mut void, r);
    return r;
}

// SSA representation: bool is i1; everything else as in memory.
fn vty(t: *mut ast.Type) *mut L.LType {
    if t.kind == ast.TY_BOOL {
        return T_i1;
    }
    return lty(t);
}

fn by_ptr(t: *mut ast.Type) bool {
    return ty.is_aggregate(t) and (ty.type_size(t) > 32);
}

struct FnLty {
    t: *mut L.LType,
    sret: bool,
}

fn fn_lty(fnty: *mut ast.Type) FnLty {
    let ret = fnty.ret;
    let sret = by_ptr(ret);
    let mut n = fnty.params.len;
    if sret {
        n += 1;
    }
    let ps = u.newn.[*mut L.LType](n + 1);
    let mut k: usize = 0;
    if sret {
        ps[k] = T_ptr;
        k += 1;
    }
    for i in 0..fnty.params.len {
        if by_ptr(fnty.params[i]) {
            ps[k] = T_ptr;
        } else {
            ps[k] = vty(fnty.params[i]);
        }
        k += 1;
    }
    let mut rt = T_void;
    if not sret and (ret.kind != ast.TY_VOID) {
        rt = vty(ret);
    }
    return FnLty.{ t: fn_type(rt, ps[0..n], false), sret: sret };
}

// ---------------------------------------------------------------------------
// Small IR helpers
// ---------------------------------------------------------------------------
fn cint(t: *mut L.LType, v: u.I128) *mut L.Value {
    if L.LLVMGetIntTypeWidth(t) <= 64 {
        return L.LLVMConstInt(t, v.lo, 1);
    }
    let words: [2]u64 = [v.lo, v.hi];
    return L.LLVMConstIntOfArbitraryPrecision(t, 2, &words[0]);
}

fn usz(v: i64) *mut L.Value {
    return L.LLVMConstInt(T_isz, @bit_cast(u64, v), 0);
}

fn terminated() bool {
    let bb = L.LLVMGetInsertBlock(B);
    return u.notnil(L.LLVMGetBasicBlockTerminator(bb));
}

fn new_bb(name: *u8) *mut L.BB {
    return L.LLVMAppendBasicBlockInContext(ctx, F.fnv, name);
}

// After a diverging instruction: continue in a fresh (unreachable) block.
fn dead_block() void {
    at_end(new_bb("dead".ptr));
}

fn intrinsic(name: []u8, tys: []mut *mut L.LType, args: []mut *mut L.Value) *mut L.Value {
    let id = L.LLVMLookupIntrinsicID(u.cz(name), name.len);
    if id == 0 {
        internal_error(u.f1("unknown intrinsic %s", name));
    }
    let mut tp = u.nil.[*mut L.LType]();
    if tys.len > 0 {
        tp = &mut tys[0];
    }
    let f = L.LLVMGetIntrinsicDeclaration(M, id, tp, tys.len);
    let fty = L.LLVMIntrinsicGetType(ctx, id, tp, tys.len);
    return call(fty, f, args);
}

fn no_tys() []mut *mut L.LType {
    return @slice_from_ptr(u.nil.[*mut L.LType](), 0);
}

fn no_vals() []mut *mut L.Value {
    return @slice_from_ptr(u.nil.[*mut L.Value](), 0);
}

fn intr1(name: []u8, a: *mut L.Value) *mut L.Value {
    let mut t: [1]*mut L.LType = [L.LLVMTypeOf(a)];
    let mut args: [1]*mut L.Value = [a];
    return intrinsic(name, t[..], args[..]);
}

fn intr2(name: []u8, a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    let mut t: [1]*mut L.LType = [L.LLVMTypeOf(a)];
    let mut args: [2]*mut L.Value = [a, b];
    return intrinsic(name, t[..], args[..]);
}

// ---------------------------------------------------------------------------
// Traps (spec Appendix B).
//
// Every trap site has a constant record {code, file id, span, line, column,
// file name, source text}. Before trapping, the helper stores the record's
// address (and, for @expect_eq, the compared values) in phalange.trap_info,
// which hosts read after the trap (wasm export phalange_trap_info; native
// weak hook ph_on_trap). Going through one cold helper also keeps LLVM's fast
// instruction selector usable at -O0; -O1+ inlines it.
// ---------------------------------------------------------------------------
const T_TRAP: i32 = 0;
const T_BOUNDS: i32 = 1;
const T_OVERFLOW: i32 = 2;
const T_DIVZERO: i32 = 3;
const T_INTCAST: i32 = 4;
const T_LENGTH: i32 = 6;
const T_UNIONTAG: i32 = 7;
const T_ATOMIC_ALIGN: i32 = 10;
const T_EXPECT: i32 = 11;
const SITE_ERR_RETURN: i32 = 254;
const SITE_CALL: i32 = 255;  // not traps: shadow stack records

let mut g_cur_span: u.Span = u.Span.{ file: 0, start: 0, end: 0 };
let mut trap_fn: *mut L.Value = @ptr_from_int(*mut L.Value, 0);
let mut trap_info: *mut L.Value = @ptr_from_int(*mut L.Value, 0);
let mut trap_hook: *mut L.Value = @ptr_from_int(*mut L.Value, 0);
let mut trap_fnty: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut site_ty: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut site_map: u.SymMap.[*mut L.Value] = u.SymMap.[*mut L.Value].{
    keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut *mut L.Value, 0), cap: 0, len: 0 };

// phalange.trap_info: {site, kind, a, b, alen, blen}, all 64-bit.
fn trap_info_global() *mut L.Value {
    if u.isnil(trap_info) {
        let t = L.LLVMArrayType2(T_i64, 6);
        trap_info = shared_global(t, "phalange.trap_info", L.LLVMConstNull(t), 0);
        L.LLVMSetAlignment(trap_info, 8);
    }
    return trap_info;
}

fn trap_info_slot(i: u64) *mut L.Value {
    return gep2(L.LLVMArrayType2(T_i64, 6), trap_info_global(), ci32(0), ci32(i));
}

// Stable identity of a source file in site records (hash of its display path).
pub fn file_key(file_id: i32) u32 {
    if (file_id < 0) or (@int_cast(usize, file_id) >= u.g_files.len) {
        return 0xFFFF_FFFF;
    }
    let mut h: u32 = 2166136261;
    for c in u.file_of(file_id).display {
        h = (h ^ @as(u32, c)) *% 16777619;
    }
    if h == 0xFFFF_FFFF {
        return 0;
    }
    return h;
}

// Exported symbol names of the whole program (for the linker), and libraries
// named by extern "C" blocks: computed from the checked program.
pub fn collect_link_names() void {
    let mut seen = u.SymMap.[i32].{ keys: u.nil.[usize](), vals: u.nil.[i32](), cap: 0, len: 0 };
    let P = sema.g_prog;
    for i in 0..P.fns.len {
        let d = P.fns.get(i);
        if not d.is_export {
            continue;
        }
        let s = u.intern(export_symbol(d));
        if seen.get(s) == 0 {
            g_export_names.push(s);
        }
        seen.put(s, 1);
    }
    for i in 0..P.host_fns.len {
        let d = P.host_fns.get(i);
        if not d.extern_c or u.isnos(d.link_lib) {
            continue;
        }
        let s = u.intern(d.link_lib);
        if seen.get(s) == 0 {
            g_link_libs.push(s);
        }
        seen.put(s, 1);
    }
}

fn cur_fn_display() []u8 {
    if u.notnil(F) and u.notnil(F.inst) {
        return F.inst.display;
    }
    return "";
}

// A constant site record {code, file id, start, end, line, col, file, text,
// fn, callee}. `fn` is the enclosing function; `callee` is set for call sites.
// Codes: a trap code, SITE_CALL for a call, SITE_ERR_RETURN for an error return.
fn site_record_ex(code: i32, sp: u.Span, callee0: []u8) *mut L.Value {
    let fname = cur_fn_display();
    let mut callee = callee0;
    if u.isnos(callee) {
        callee = "";
    }
    let key = u.intern(u.f6("%s:%s:%s:%s:%s:%s", u.idec(code), u.idec(sp.file), u.udec(@as(u64, sp.start)),
                            u.udec(@as(u64, sp.end)), fname, callee));
    let found = site_map.get(key);
    if u.notnil(found) {
        return found;
    }
    if u.isnil(site_ty) {
        let mut el: [10]*mut L.LType = [T_i32, T_i32, T_i32, T_i32, T_i32, T_i32, T_ptr, T_ptr, T_ptr, T_ptr];
        site_ty = struct_ty(el[..]);
    }
    let mut lc = u.LineCol.{ sl: 0, sc: 0, el: 0, ec: 0 };
    let mut file: []u8 = "";
    let mut text: []u8 = "";
    if (sp.file >= 0) and (@int_cast(usize, sp.file) < u.g_files.len) and (sp.end >= sp.start) {
        lc = u.span_linecol(sp);
        let f = u.file_of(sp.file);
        file = f.display;
        let mut n = sp.end - sp.start;
        let mut max: u32 = 200;
        if code >= SITE_ERR_RETURN {
            max = 80;
        }
        if n > max {
            n = max;
        }
        // one line of text only
        for i in 0..n {
            if f.text[@as(usize, sp.start + i)] == '\n' {
                n = i;
                break;
            }
        }
        text = u.dup(f.text[@as(usize, sp.start)..@as(usize, sp.start + n)]);
    }
    let mut vals: [10]*mut L.Value = [
        ci32(@as(u64, @bit_cast(u32, code))), ci32(@as(u64, file_key(sp.file))),
        ci32(@as(u64, sp.start)), ci32(@as(u64, sp.end)),
        ci32(@as(u64, @bit_cast(u32, lc.sl))), ci32(@as(u64, @bit_cast(u32, lc.sc))),
        string_ptr(cstr_prefix(file)), string_ptr(cstr_prefix(text)),
        string_ptr(cstr_prefix(fname)), string_ptr(cstr_prefix(callee)),
    ];
    let g = L.LLVMAddGlobal(M, site_ty, ".site".ptr);
    L.LLVMSetInitializer(g, const_struct(vals[..]));
    L.LLVMSetLinkage(g, L.PrivateLinkage);
    L.LLVMSetGlobalConstant(g, 1);
    L.LLVMSetAlignment(g, 8);
    site_map.put(key, g);
    return g;
}

// The bytes up to the first NUL (C strings in the C implementation).
fn cstr_prefix(s: []u8) []u8 {
    let z = u.find(s, 0);
    if z >= 0 {
        return s[0..@int_cast(usize, z)];
    }
    return s;
}

fn site_record(code: i32, sp: u.Span) *mut L.Value {
    return site_record_ex(code, sp, u.nosym());
}

// ---------------------------------------------------------------------------
// Shadow stack (spec §17.4): each thread has
//     {u32 depth, u32 errors, [SS_CAP]*Site calls, [ET_CAP]*Site error_returns}
// Every call stores its call-site record at calls[depth % SS_CAP] and bumps
// depth for the duration of the call, so after a trap or crash the host can
// print the innermost SS_CAP frames with exact source locations. Error
// returns are appended to a ring the same way; `catch` clears it. On wasm the
// record's address lives in a per-instance (so per-thread) wasm global; on
// native targets the record is thread-local.
// ---------------------------------------------------------------------------
const SS_CAP: u64 = 256;
const ET_CAP: u64 = 32;
let mut ss_ty: *mut L.LType = @ptr_from_int(*mut L.LType, 0);
let mut ss_global: *mut L.Value = @ptr_from_int(*mut L.Value, 0);
let mut ss_main: *mut L.Value = @ptr_from_int(*mut L.Value, 0);

fn stack_traces_on() bool {
    return u.notnil(O) and O.stack_traces;
}

fn ss_type() *mut L.LType {
    if u.isnil(ss_ty) {
        let mut el: [4]*mut L.LType = [T_i32, T_i32, L.LLVMArrayType2(T_ptr, SS_CAP), L.LLVMArrayType2(T_ptr, ET_CAP)];
        ss_ty = struct_ty(el[..]);
    }
    return ss_ty;
}

fn ss_globals() void {
    let t = ss_type();
    if u.isnil(ss_global) {
        if ty.g_target.is_wasm {
            ss_main = shared_global(t, "phalange.ss.main", L.LLVMConstNull(t), 0);
            L.LLVMSetAlignment(ss_main, 8);
            // null means the main thread's record (wasm-ld does not relocate
            // wasm global initializers); thread starts set their own.
            ss_global = shared_global(T_ptr, "phalange.ss", L.LLVMConstNull(T_ptr), 1);
        } else {
            ss_global = shared_global(t, "phalange.ss", L.LLVMConstNull(t), 0);
            L.LLVMSetThreadLocal(ss_global, 1);
            L.LLVMSetThreadLocalMode(ss_global, L.InitialExecTLSModel);
            L.LLVMSetAlignment(ss_global, 8);
        }
    }
}

fn ss_ptr() *mut L.Value {
    ss_globals();
    if ty.g_target.is_wasm {
        let p = ld(T_ptr, ss_global);
        return L.LLVMBuildSelect(B, L.LLVMBuildIsNull(B, p, NN()), ss_main, p, "ss".ptr);
    }
    return ss_global;
}

fn ss_field(ss: *mut L.Value, i: u32) *mut L.Value {
    return sgep(ss_type(), ss, i);
}

fn ss_slot(ss: *mut L.Value, field: u64, idx: *mut L.Value) *mut L.Value {
    let mut ix: [3]*mut L.Value = [ci32(0), ci32(field), idx];
    return L.LLVMBuildGEP2(B, ss_type(), ss, &mut ix[0], 3, NN());
}

struct SsFrame {
    ss: *mut L.Value,
    depth: *mut L.Value,
}

// In profiling builds another thread (or a signal handler) reads the stack at
// any moment, so the stores must happen even where nothing in this thread
// reads them (after inlining, an increment followed by the restore is
// otherwise a dead store).
fn ss_store(v: *mut L.Value, p: *mut L.Value) void {
    let s = st(v, p);
    if O.profile {
        L.LLVMSetVolatile(s, 1);
    }
}

fn ss_push(callee: []u8) SsFrame {
    let mut f = SsFrame.{ ss: vnil(), depth: vnil() };
    if not stack_traces_on() or u.isnil(F) {
        return f;
    }
    f.ss = ss_ptr();
    let dp = ss_field(f.ss, 0);
    f.depth = L.LLVMBuildLoad2(B, T_i32, dp, "depth".ptr);
    let idx = b_and(f.depth, ci32(SS_CAP - 1));
    ss_store(site_record_ex(SITE_CALL, g_cur_span, callee), ss_slot(f.ss, 2, idx));
    ss_store(b_add(f.depth, ci32(1)), dp);
    return f;
}

fn ss_pop(f: SsFrame) void {
    if u.notnil(f.ss) {
        ss_store(f.depth, ss_field(f.ss, 0));
    }
}

// Appends an error-return record when `code` (an error code, i16) is nonzero.
fn etrace_push(code: *mut L.Value) void {
    if not stack_traces_on() or u.isnil(F) {
        return;
    }
    if is_const_int(code) and (L.LLVMConstIntGetZExtValue(code) == 0) {
        return;
    }
    let mut cont = bbnil();
    if not is_const_int(code) {
        let rec = new_bb("etrace".ptr);
        cont = new_bb("etrace.done".ptr);
        condbr(icmp(L.IntNE, code, L.LLVMConstNull(L.LLVMTypeOf(code))), rec, cont);
        at_end(rec);
    }
    let ss = ss_ptr();
    let np = ss_field(ss, 1);
    let n = ld(T_i32, np);
    let idx = b_and(n, ci32(ET_CAP - 1));
    _ = st(site_record(SITE_ERR_RETURN, g_cur_span), ss_slot(ss, 3, idx));
    _ = st(b_add(n, ci32(1)), np);
    if u.notnil(cont) {
        br(cont);
        at_end(cont);
    }
}

fn etrace_clear() void {
    if not stack_traces_on() or u.isnil(F) {
        return;
    }
    _ = st(ci32(0), ss_field(ss_ptr(), 1));
}

fn add_fn_attr(f: *mut L.Value, name: []u8) void {
    let k = L.LLVMGetEnumAttributeKindForName(u.cz(name), name.len);
    L.LLVMAddAttributeAtIndex(f, L.FunctionIndex, L.LLVMCreateEnumAttribute(ctx, k, 0));
}

fn add_str_attr(f: *mut L.Value, k: []u8, v: []u8) void {
    L.LLVMAddAttributeAtIndex(f, L.FunctionIndex, L.LLVMCreateStringAttribute(ctx, u.cz(k), @int_cast(u32, k.len), u.cz(v),
                                                                             @int_cast(u32, v.len)));
}

fn call_trap_site(site: *mut L.Value) void {
    if u.isnil(trap_fn) {
        let mut ps: [1]*mut L.LType = [T_ptr];
        trap_fnty = fn_type(T_void, ps[..], false);
        trap_fn = L.LLVMAddFunction(M, "phalange.trap".ptr, trap_fnty);
        L.LLVMSetLinkage(trap_fn, L.InternalLinkage);
        add_fn_attr(trap_fn, "noreturn");
        add_fn_attr(trap_fn, "cold");
        add_fn_attr(trap_fn, "nounwind");
        add_fn_attr(trap_fn, "noinline");
        let cur = L.LLVMGetInsertBlock(B);
        let f = trap_fn;
        at_end(L.LLVMAppendBasicBlockInContext(ctx, f, "entry".ptr));
        let sp = L.LLVMGetParam(f, 0);
        _ = st(zext(ptrtoint(sp, T_isz), T_i64), trap_info_slot(0));
        if not ty.g_target.is_wasm {
            // Optional host hook (runtime/native_host.c, runtime/test_main.c).
            let mut hps: [3]*mut L.LType = [T_ptr, T_ptr, T_ptr];
            let hty = fn_type(T_void, hps[..], false);
            trap_hook = L.LLVMAddFunction(M, "ph_on_trap".ptr, hty);
            L.LLVMSetLinkage(trap_hook, L.ExternalWeakLinkage);
            let call_bb = L.LLVMAppendBasicBlockInContext(ctx, f, "hook".ptr);
            let done_bb = L.LLVMAppendBasicBlockInContext(ctx, f, "trap".ptr);
            let has = icmp(L.IntNE, trap_hook, L.LLVMConstNull(T_ptr));
            condbr(has, call_bb, done_bb);
            at_end(call_bb);
            let mut ssv = L.LLVMConstNull(T_ptr);
            if stack_traces_on() {
                ssv = ss_ptr();
            }
            let mut args: [3]*mut L.Value = [sp, trap_info_global(), ssv];
            _ = call(hty, trap_hook, args[..]);
            br(done_bb);
            at_end(done_bb);
        }
        _ = intrinsic("llvm.trap", no_tys(), no_vals());
        _ = L.LLVMBuildUnreachable(B);
        at_end(cur);
    }
    let mut a: [1]*mut L.Value = [site];
    _ = call(trap_fnty, trap_fn, a[..]);
}

// A block that traps with `code` at the current source span.
fn trap_block(code: i32) *mut L.BB {
    let cur = L.LLVMGetInsertBlock(B);
    let bb = new_bb("trap".ptr);
    at_end(bb);
    call_trap_site(site_record(code, g_cur_span));
    _ = L.LLVMBuildUnreachable(B);
    at_end(cur);
    return bb;
}

// Continue only if `ok` holds; otherwise trap with `code`.
fn check_or_trap(ok: *mut L.Value, code: i32) void {
    if is_const_int(ok) and (L.LLVMConstIntGetZExtValue(ok) == 1) {
        return;
    }
    let cont = new_bb("ok".ptr);
    condbr(ok, cont, trap_block(code));
    at_end(cont);
}

fn emit_trap(code: i32) void {
    call_trap_site(site_record(code, g_cur_span));
    _ = L.LLVMBuildUnreachable(B);
    dead_block();
}

fn alloca_in_entry(t: *mut L.LType, name: *u8) *mut L.Value {
    let cur = L.LLVMGetInsertBlock(B);
    let term = L.LLVMGetBasicBlockTerminator(F.entry);
    if u.notnil(term) {
        L.LLVMPositionBuilderBefore(B, term);
    } else {
        at_end(F.entry);
    }
    let a = L.LLVMBuildAlloca(B, t, name);
    at_end(cur);
    return a;
}

fn temp_alloca(t: *mut ast.Type) *mut L.Value {
    let a = alloca_in_entry(lty(t), "tmp".ptr);
    L.LLVMSetAlignment(a, @int_cast(u32, ty.type_align(t)));
    return a;
}

fn to_mem(t: *mut ast.Type, v: *mut L.Value) *mut L.Value {
    if t.kind == ast.TY_BOOL {
        return zext(v, T_i8);
    }
    return v;
}

fn from_mem(t: *mut ast.Type, v: *mut L.Value) *mut L.Value {
    if t.kind == ast.TY_BOOL {
        return icmp(L.IntNE, v, ci(T_i8, 0));
    }
    return v;
}

// unaligned: the storage may be misaligned (reached through pointers)
fn load(t: *mut ast.Type, p: *mut L.Value, unaligned: bool) *mut L.Value {
    let v = ld(lty(t), p);
    if unaligned {
        L.LLVMSetAlignment(v, 1);
    }
    return from_mem(t, v);
}

fn store(t: *mut ast.Type, v: *mut L.Value, p: *mut L.Value, unaligned: bool) void {
    let s = st(to_mem(t, v), p);
    if unaligned {
        L.LLVMSetAlignment(s, 1);
    }
}

fn spill(t: *mut ast.Type, v: *mut L.Value) *mut L.Value {
    let a = temp_alloca(t);
    store(t, v, a, false);
    return a;
}

fn local_ptr(l: *mut ast.Local) *mut L.Value {
    let i = @int_cast(usize, l.index);
    if u.isnil(F.locals[i]) {
        let t = ct(l.ty);
        let a = alloca_in_entry(lty(t), u.cz(l.name));
        L.LLVMSetAlignment(a, @int_cast(u32, ty.type_align(t)));
        F.locals[i] = a;
    }
    return F.locals[i];
}

// ---------------------------------------------------------------------------
// Strings and globals
// ---------------------------------------------------------------------------
let mut string_globals: u.SymMap.[*mut L.Value] = u.SymMap.[*mut L.Value].{
    keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut *mut L.Value, 0), cap: 0, len: 0 };

fn string_ptr(bytes: []u8) *mut L.Value {
    let key = u.intern(bytes);
    let mut g = string_globals.get(key);
    if u.isnil(g) {
        // A NUL follows every literal (not part of the slice), so that
        // "text".ptr is a valid C string (spec §2.5).
        let init = L.LLVMConstStringInContext(ctx, key.ptr, @int_cast(u32, bytes.len), 0);
        g = L.LLVMAddGlobal(M, L.LLVMArrayType2(T_i8, @as(u64, bytes.len) + 1), ".str".ptr);
        L.LLVMSetInitializer(g, init);
        L.LLVMSetLinkage(g, L.PrivateLinkage);
        L.LLVMSetGlobalConstant(g, 1);
        L.LLVMSetUnnamedAddress(g, L.GlobalUnnamedAddr);
        L.LLVMSetAlignment(g, 1);
        string_globals.put(key, g);
    }
    return g;
}

fn const_slice(ptr: *mut L.Value, len: i64) *mut L.Value {
    let mut el: [2]*mut L.Value = [ptr, usz(len)];
    return const_struct(el[..]);
}

let mut global_vals: u.SymMap.[*mut L.Value] = u.SymMap.[*mut L.Value].{
    keys: @ptr_from_int(*mut usize, 0), vals: @ptr_from_int(*mut *mut L.Value, 0), cap: 0, len: 0 };

fn const_struct2(a: *mut L.Value, b: *mut L.Value) *mut L.Value {
    let mut el: [2]*mut L.Value = [a, b];
    return const_struct(el[..]);
}

fn const_struct1(a: *mut L.Value) *mut L.Value {
    let mut el: [1]*mut L.Value = [a];
    return const_struct(el[..]);
}

// Constant for a global initializer (spec: constant expressions only).
fn gen_const(e: *mut ast.Expr, t: *mut ast.Type) *mut L.Value {
    if e.kind == ast.E_CONV {
        if e.op == ast.CONV_NOOP {
            return gen_const(e.a, t);
        }
        if e.op == ast.CONV_WRAP_OK {
            if t.elem.kind == ast.TY_VOID {
                return const_struct1(ci(T_i16, 0));
            }
            return const_struct2(ci(T_i16, 0), gen_const(e.a, t.elem));
        }
        if e.op == ast.CONV_WRAP_ERR {
            let code = gen_const(e.a, ty.t_error);
            if t.elem.kind == ast.TY_VOID {
                return const_struct1(code);
            }
            return const_struct2(code, L.LLVMConstNull(lty(t.elem)));
        }
    }
    if e.is_const and (t.kind == ast.TY_INT) {
        return cint(lty(t), e.cval);
    }
    if e.is_const and (t.kind == ast.TY_FLOAT) {
        return L.LLVMConstReal(lty(t), e.fval);
    }
    if (t.kind == ast.TY_BOOL) and ((e.kind == ast.E_BOOL) or e.is_const) {
        let mut bv: u64 = 0;
        if e.bval {
            bv = 1;
        }
        return ci(T_i8, bv);
    }
    let k = e.kind;
    if k == ast.E_FLOAT {
        return L.LLVMConstReal(lty(t), e.fval);
    }
    if k == ast.E_STRING {
        return const_slice(string_ptr(e.str), @int_cast(i64, e.str.len));
    }
    if k == ast.E_ERROR_VAL {
        return ci(T_i16, e.cval.lo);
    }
    if k == ast.E_ARRAY_LIT {
        let n = e.args.len;
        let vs = u.newn.[*mut L.Value](n + 1);
        for i in 0..n {
            vs[i] = gen_const(e.args.get(i), t.elem);
        }
        return L.LLVMConstArray2(lty(t.elem), &mut vs[0], @as(u64, n));
    }
    if k == ast.E_ARRAY_REPEAT {
        let v = gen_const(e.a, t.elem);
        if L.LLVMIsNull(v) != 0 {
            return L.LLVMConstNull(lty(t));
        }
        let n = @int_cast(usize, t.len);
        let vs = u.newn.[*mut L.Value](n + 1);
        for i in 0..n {
            vs[i] = v;
        }
        return L.LLVMConstArray2(lty(t.elem), &mut vs[0], @as(u64, n));
    }
    if k == ast.E_STRUCT_LIT {
        let n = t.decl.fields.len;
        let vs = u.newn.[*mut L.Value](n + 1);
        for i in 0..n {
            vs[i] = gen_const(e.inits.at(i).value, ty.type_field(t, i));
        }
        return L.LLVMConstNamedStruct(lty(t), &mut vs[0], @int_cast(u32, n));
    }
    if k == ast.E_FIELD {
        if e.op == ast.FK_VARIANT_UNIT {
            let ut = lty(t);
            let tag = ci(lty(ty.union_tag_type(t)), @int_cast(u64, e.index));
            if L.LLVMCountStructElementTypes(ut) == 1 {
                let mut one: [1]*mut L.Value = [tag];
                return L.LLVMConstNamedStruct(ut, &mut one[0], 1);
            }
            let mut el: [2]*mut L.Value = [tag, L.LLVMConstNull(L.LLVMStructGetTypeAtIndex(ut, 1))];
            return L.LLVMConstNamedStruct(ut, &mut el[0], 2);
        }
        if (e.op == ast.FK_DECL) and u.notnil(e.decl) and (e.decl.kind == ast.D_FN) {
            return get_inst(e.decl, ty.no_types()).fnv;
        }
    } else if k == ast.E_IDENT {
        if u.notnil(e.decl) and ((e.decl.kind == ast.D_FN) or (e.decl.kind == ast.D_HOST_FN)) {
            return get_inst(e.decl, ty.no_types()).fnv;
        }
    } else if k == ast.E_BUILTIN {
        if e.index == ast.BI_PTR_FROM_INT {
            return L.LLVMConstIntToPtr(gen_const(e.bargs.at(1).e, ty.t_usize), T_ptr);
        }
    } else if k == ast.E_UNARY {
        if (e.op == lx.TK_MINUS) and (t.kind == ast.TY_FLOAT) {
            return L.LLVMConstReal(lty(t), -e.a.fval);
        }
    } else if k == ast.E_CALL {
        if e.op == ast.CALLK_VARIANT {
            _ = dg.err(e.span, "E0353", "union constants with a payload are not supported by this compiler yet");
            return L.LLVMConstNull(lty(t));
        }
    }
    _ = dg.err(e.span, "E0350", "global initializer must be a constant expression");
    return L.LLVMConstNull(lty(t));
}

// Symbols are prefixed with the module's path (without .ph), which is unique
// within a build, so modules compiled separately never clash.
fn mangle_prefix(d: *mut ast.Decl) []u8 {
    let mut base: []u8 = "";
    if u.notnil(d.module) {
        base = u.file_of(d.module.file.id).display;
    }
    if (base.len > 3) and u.ends(base, ".ph") {
        return u.dup(base[0..base.len - 3]);
    }
    return u.dup(base);
}

// A type in a symbol name: like type_str, but struct and union names are
// qualified by their module, so instances are named the same in every unit.
fn mangle_type_into(b: *mut u.Buf, t: *mut ast.Type) void {
    let k = t.kind;
    if (k == ast.TY_STRUCT) or (k == ast.TY_UNION) {
        if t.decl.anon {  // anonymous structs are the same type everywhere
            b.s("struct{");
            for i in 0..t.decl.fields.len {
                if i != 0 {
                    b.s(",");
                }
                b.s(t.decl.fields.at(i).name);
                b.s(":");
                mangle_type_into(b, ty.type_field(t, i));
            }
            b.s("}");
            return;
        }
        b.s(mangle_prefix(t.decl));
        b.s(".");
        b.s(t.decl.name);
        if t.args.len != 0 {
            b.s("[");
            for i in 0..t.args.len {
                if i != 0 {
                    b.s(",");
                }
                mangle_type_into(b, t.args[i]);
            }
            b.s("]");
        }
    } else if k == ast.TY_PTR {
        b.s(u.pick(t.is_mut, "*mut ", "*"));
        mangle_type_into(b, t.elem);
    } else if k == ast.TY_SLICE {
        b.s(u.pick(t.is_mut, "[]mut ", "[]"));
        mangle_type_into(b, t.elem);
    } else if k == ast.TY_ARRAY {
        b.s("[");
        b.s(u.dec(t.len));
        b.s("]");
        mangle_type_into(b, t.elem);
    } else if k == ast.TY_ERRUNION {
        b.s("!");
        mangle_type_into(b, t.elem);
    } else if k == ast.TY_FN {
        b.s("fn(");
        for i in 0..t.params.len {
            if i != 0 {
                b.s(",");
            }
            mangle_type_into(b, t.params[i]);
        }
        b.s(")");
        mangle_type_into(b, t.ret);
    } else {
        b.s(ty.type_str(t));
    }
}

fn mangle_type(t: *mut ast.Type) []u8 {
    let mut b = u.Buf.init();
    mangle_type_into(&mut b, t);
    return b.done();
}

// Owner.[A].name.[B], or test "name".
fn fn_display(d: *mut ast.Decl, gargs: []mut *mut ast.Type) []u8 {
    if d.is_test {
        return u.f1("test \"%s\"", d.test_name);
    }
    let n = gargs.len;
    let mut b = u.Buf.init();
    let mut nowner: usize = 0;
    if u.notnil(d.owner) {
        nowner = d.owner.gparams.len;
        b.s(d.owner.name);
        if nowner != 0 {
            b.s(".[");
            for i in 0..nowner {
                if i >= n {
                    break;
                }
                if i != 0 {
                    b.s(", ");
                }
                b.s(ty.type_str(gargs[i]));
            }
            b.s("]");
        }
        b.s(".");
    }
    b.s(d.name);
    let mut nown = d.gparams.len;
    if d.kind == ast.D_HOST_FN {
        nown = 0;
    }
    if nown != 0 {
        b.s(".[");
        for i in n - nown..n {
            if i > (n - nown) {
                b.s(", ");
            }
            b.s(ty.type_str(gargs[i]));
        }
        b.s("]");
    }
    return b.done();
}

// The symbol of an exported function. On wasm (and in test binaries, which
// have their own main) `main` is renamed; the export name is an attribute.
fn export_symbol(d: *mut ast.Decl) []u8 {
    if (ty.g_target.is_wasm or g_test_mode) and u.str_eq(d.name, "main") {
        return "phalange.export.main";
    }
    return d.name;
}

// Globals live in the link unit when units are compiled separately (their
// initializers may contain error codes, which only the link unit knows).
fn global_ref(d: *mut ast.Decl) *mut L.Value {
    let k = u.addr_key(@int_from_ptr(d));
    let found = global_vals.get(k);
    if u.notnil(found) {
        return found;
    }
    let t = d.ty;
    let g = L.LLVMAddGlobal(M, lty(t), u.cz(u.f2("%s.%s", mangle_prefix(d), d.name)));
    L.LLVMSetAlignment(g, @int_cast(u32, ty.type_align(t)));
    global_vals.put(k, g);
    if O.unit == CG_MODULE {
        return g;  // defined by the link unit
    }
    if O.unit == CG_WHOLE {
        L.LLVMSetLinkage(g, L.InternalLinkage);
    }
    L.LLVMSetInitializer(g, gen_const(d.init, t));
    if not d.is_mut {
        L.LLVMSetGlobalConstant(g, 1);
    }
    return g;
}

// Program-wide state shared by all units: defined (internal) in a whole
// program, declared by module units, defined (external) by the link unit.
fn shared_global(t: *mut L.LType, name: []u8, init: *mut L.Value, addrspace: u32) *mut L.Value {
    let mut g = vnil();
    if addrspace != 0 {
        g = L.LLVMAddGlobalInAddressSpace(M, t, u.cz(name), addrspace);
    } else {
        g = L.LLVMAddGlobal(M, t, u.cz(name));
    }
    if O.unit == CG_MODULE {
        return g;
    }
    L.LLVMSetInitializer(g, init);
    if O.unit == CG_WHOLE {
        L.LLVMSetLinkage(g, L.InternalLinkage);
    }
    return g;
}

// An error code: a constant, except in module units, where the codes are
// only known at link time (spec §11.1) and are loaded from the link unit.
fn errcode_value(code: u32) *mut L.Value {
    if (O.unit != CG_MODULE) or (code == 0) or (@as(usize, code) > sema.g_prog.error_names.len) {
        return ci(T_i16, @as(u64, code));
    }
    let name = u.cz(u.f1("phalange.errcode.%s", sema.g_prog.error_names.get(@as(usize, code) - 1)));
    let mut g = L.LLVMGetNamedGlobal(M, name);
    if u.isnil(g) {
        g = L.LLVMAddGlobal(M, T_i16, name);
        L.LLVMSetGlobalConstant(g, 1);
    }
    return ld(T_i16, g);
}

fn type_mentions_error(t: *mut ast.Type) bool {
    let k = t.kind;
    if (k == ast.TY_ERROR) or (k == ast.TY_ERRUNION) {
        return true;
    }
    if k == ast.TY_ARRAY {
        return type_mentions_error(t.elem);
    }
    if k == ast.TY_STRUCT {
        for i in 0..t.decl.fields.len {
            if type_mentions_error(ty.type_field(t, i)) {
                return true;
            }
        }
        return false;
    }
    if k == ast.TY_UNION {
        return true;  // conservatively
    }
    return false;
}

// ---------------------------------------------------------------------------
// Function instances
// ---------------------------------------------------------------------------
fn get_inst(d: *mut ast.Decl, gargs: []mut *mut ast.Type) *mut FnInst {
    let n = gargs.len;
    let mut kb = u.Buf.init();
    kb.s(u.udec(@as(u64, @int_from_ptr(d))));
    for i in 0..n {
        kb.s(",");
        kb.s(u.idec(gargs[i].id));
    }
    let k = u.intern(kb.done());
    let found = inst_map.get(k);
    if u.notnil(found) {
        return found;
    }
    let fi = u.new.[FnInst]();
    fi.decl = d;
    fi.gargs = gargs;
    for i in 0..n {
        if gargs[i].has_params {
            internal_error("generic instance with unsubstituted arguments");
        }
    }
    fi.fnty = d.ty;
    if n != 0 {
        fi.fnty = ty.subst(d.ty, d.all_gparams, gargs);
    }
    let fl = fn_lty(fi.fnty);
    fi.llty = fl.t;
    fi.sret = fl.sret;
    fi.display = fn_display(d, gargs);
    let mut name = d.name;
    if d.kind == ast.D_HOST_FN {
        name = d.name;
    } else if d.is_export {
        // On wasm the symbol gets a private name and the export name comes
        // from the wasm-export-name attribute: LLVM treats a symbol called
        // `main` specially on wasm (it would add a second `main` export).
        // Test binaries bring their own `main` (the test driver).
        name = export_symbol(d);
    } else {
        let mut nb = u.Buf.init();
        nb.s(mangle_prefix(d));
        nb.s(".");
        if u.notnil(d.owner) {
            nb.s(d.owner.name);
            nb.s(".");
        }
        nb.s(d.name);
        let nown = d.gparams.len;
        if nown != 0 {
            nb.s(".[");
            for i in n - nown..n {
                if i > (n - nown) {
                    nb.s(", ");
                }
                nb.s(mangle_type(gargs[i]));
            }
            nb.s("]");
        }
        if u.notnil(d.owner) and (d.owner.gparams.len != 0) {
            nb.s("@[");
            for i in 0..d.owner.gparams.len {
                if i != 0 {
                    nb.s(",");
                }
                nb.s(mangle_type(gargs[i]));
            }
            nb.s("]");
        }
        name = nb.done();
    }
    if d.variadic {
        let np = L.LLVMCountParamTypes(fi.llty);
        let pts = u.newn.[*mut L.LType](@as(usize, np) + 1);
        L.LLVMGetParamTypes(fi.llty, &mut pts[0]);
        fi.llty = fn_type(L.LLVMGetReturnType(fi.llty), pts[0..@as(usize, np)], true);
    }
    let mut existing = vnil();
    if d.kind == ast.D_HOST_FN {
        existing = L.LLVMGetNamedFunction(M, u.cz(name));
    }
    if u.notnil(existing) {
        fi.fnv = existing;
    } else {
        fi.fnv = L.LLVMAddFunction(M, u.cz(name), fi.llty);
    }
    add_ext_attrs(fi.fnv, fi.fnty, fi.sret, false);
    if d.kind == ast.D_HOST_FN {
        if d.extern_c {
            // libraries: see collect_link_names
        } else if ty.g_target.is_wasm {
            add_str_attr(fi.fnv, "wasm-import-module", d.host_module);
            add_str_attr(fi.fnv, "wasm-import-name", d.name);
        }
    } else {
        // Whole program: everything but exports is internal. Separate units:
        // a non-generic function is defined (externally visible) by its own
        // module's unit and only declared elsewhere; generic instances are
        // defined link-once by every unit that uses them.
        let mut define = true;
        if O.unit == CG_LINK {
            define = false;
        } else if (O.unit == CG_MODULE) and (n == 0) {
            define = d.module == O.module;
        }
        if (O.unit == CG_WHOLE) and not d.is_export {
            L.LLVMSetLinkage(fi.fnv, L.InternalLinkage);
        } else if define and (n > 0) {
            L.LLVMSetLinkage(fi.fnv, L.LinkOnceODRLinkage);
        }
        if define and d.is_export and ty.g_target.is_wasm {
            add_str_attr(fi.fnv, "wasm-export-name", d.name);
        }
        if define {
            if u.notnil(pending_tail) {
                pending_tail.next = fi;
            } else {
                pending_head = fi;
            }
            pending_tail = fi;
        }
    }
    inst_map.put(k, fi);
    return fi;
}

// C ABI detail: integers narrower than 32 bits and bool are passed and
// returned sign- or zero-extended. Marking every function (and indirect call)
// this way makes any Phalange function with a scalar signature callable from
// C, and C function pointers callable from Phalange (spec §15.5).
fn ext_attr(v: *mut L.Value, idx: u32, t: *mut ast.Type, is_call: bool) void {
    let mut kind = u.nosym();
    if t.kind == ast.TY_BOOL {
        kind = "zeroext";
    } else if (t.kind == ast.TY_INT) and (t.bits < 32) {
        kind = u.pick(t.is_signed, "signext", "zeroext");
    }
    if u.isnos(kind) {
        return;
    }
    let a = L.LLVMCreateEnumAttribute(ctx, L.LLVMGetEnumAttributeKindForName(kind.ptr, kind.len), 0);
    if is_call {
        L.LLVMAddCallSiteAttribute(v, idx, a);
    } else {
        L.LLVMAddAttributeAtIndex(v, idx, a);
    }
}

fn add_ext_attrs(v: *mut L.Value, fnty: *mut ast.Type, sret: bool, is_call: bool) void {
    if not sret and (fnty.ret.kind != ast.TY_VOID) {
        ext_attr(v, L.ReturnIndex, fnty.ret, is_call);
    }
    let mut first: u32 = 1;
    if sret {
        first = 2;
    }
    for i in 0..fnty.params.len {
        if not by_ptr(fnty.params[i]) {
            ext_attr(v, first + @int_cast(u32, i), fnty.params[i], is_call);
        }
    }
}

fn subst_gargs(g: []mut *mut ast.Type) []mut *mut ast.Type {
    if g.len == 0 {
        return ty.no_types();
    }
    let r = u.newn.[*mut ast.Type](g.len);
    for i in 0..g.len {
        r[i] = ct(g[i]);
    }
    return r;
}

// ---------------------------------------------------------------------------
// Defers and control flow
// ---------------------------------------------------------------------------
fn emit_defers(down_to: usize) void {
    let mut i = F.defers.len;
    while i > down_to {
        i -= 1;
        if terminated() {
            return;
        }
        let s = F.defers.get(i);
        let saved = F.defers.len;
        F.defers.len = i;  // a deferred statement runs with only the outer defers pending
        gen_stmt(s);
        F.defers.len = saved;
    }
}

fn emit_ret(v: *mut L.Value) void {
    if u.notnil(F.sret_ptr) {
        store(F.ret, v, F.sret_ptr, false);
        _ = L.LLVMBuildRetVoid(B);
    } else if F.ret.kind == ast.TY_VOID {
        _ = L.LLVMBuildRetVoid(B);
    } else {
        _ = L.LLVMBuildRet(B, v);
    }
}

fn errunion_value(eu: *mut ast.Type, code: *mut L.Value, payload: *mut L.Value) *mut L.Value {
    let t = lty(eu);
    let mut v = L.LLVMGetUndef(t);
    v = insert(v, code, 0);
    if eu.elem.kind != ast.TY_VOID {
        if u.notnil(payload) {
            v = insert(v, to_mem(eu.elem, payload), 1);
        } else {
            v = insert(v, L.LLVMConstNull(lty(eu.elem)), 1);
        }
    }
    return v;
}

fn gen_return(e: *mut ast.Expr) void {
    let mut v = vnil();
    if u.notnil(e) {
        v = gen(e);
        if terminated() {
            return;
        }
        if F.ret.kind == ast.TY_ERRUNION {
            etrace_push(extract(v, 0));
        }
    } else if F.ret.kind == ast.TY_ERRUNION {
        v = errunion_value(F.ret, ci(T_i16, 0), vnil());
    }
    emit_defers(0);
    if terminated() {
        return;
    }
    emit_ret(v);
    dead_block();
}

fn gen_break(is_break: bool) void {
    if F.loops.len == 0 {
        return;
    }
    let lc = F.loops.last();
    emit_defers(lc.defer_height);
    if is_break {
        br(lc.brk);
    } else {
        br(lc.cont);
    }
    dead_block();
}

// ---------------------------------------------------------------------------
// Arithmetic
// ---------------------------------------------------------------------------
fn int_min(t: *mut L.LType) *mut L.Value {
    let bits = L.LLVMGetIntTypeWidth(t);
    return cint(t, u.I128.pow2(bits - 1).neg());
}

fn int_max(t: *mut L.LType, is_signed: bool) *mut L.Value {
    let bits = L.LLVMGetIntTypeWidth(t);
    if is_signed {
        return cint(t, u.I128.pow2(bits - 1).sub(u.I128.of(1)));
    }
    return cint(t, u.I128.pow2(bits).sub(u.I128.of(1)));
}

struct Ovf {
    val: *mut L.Value,
    ovf: *mut L.Value,
}

// Overflow-checked arithmetic, expressed with plain instructions only.
// (The llvm.*.with.overflow intrinsics would force LLVM's slow SelectionDAG
// path at -O0 on wasm; at -O1+ LLVM re-forms the intrinsics from these
// patterns anyway.)
fn mul_wide_overflow(is_signed: bool, l: *mut L.Value, r: *mut L.Value) Ovf {
    let t = L.LLVMTypeOf(l);
    let bits = L.LLVMGetIntTypeWidth(t);
    let w = L.LLVMIntTypeInContext(ctx, bits * 2);
    let mut lw = vnil();
    let mut rw = vnil();
    if is_signed {
        lw = sext(l, w);
        rw = sext(r, w);
    } else {
        lw = zext(l, w);
        rw = zext(r, w);
    }
    let p = b_mul(lw, rw);
    let val = trunc(p, t);
    let mut back = vnil();
    if is_signed {
        back = sext(val, w);
    } else {
        back = zext(val, w);
    }
    return Ovf.{ val: val, ovf: icmp(L.IntNE, back, p) };
}

// 64-bit multiply: no 128-bit multiply needed (freestanding wasm has no
// compiler-rt); overflow is detected by dividing back.
fn mul64_overflow(is_signed: bool, l: *mut L.Value, r: *mut L.Value) Ovf {
    let t = L.LLVMTypeOf(l);
    let zero = L.LLVMConstNull(t);
    let one = ci(t, 1);
    let prod = b_mul(l, r);
    let lz = icmp(L.IntEQ, l, zero);
    if not is_signed {
        let div = sel(lz, one, l);
        let back = L.LLVMBuildUDiv(B, prod, div, NN());
        let ne = icmp(L.IntNE, back, r);  // (right to left, as in the C implementation)
        let ovf = b_and(b_not(lz), ne);
        return Ovf.{ val: prod, ovf: ovf };
    }
    let lm1 = icmp(L.IntEQ, l, L.LLVMConstAllOnes(t));
    let safe = b_or(lz, lm1);
    let div = sel(safe, one, l);
    let back = L.LLVMBuildSDiv(B, prod, div, NN());
    let ne = icmp(L.IntNE, back, r);  // (right to left, as in the C implementation)
    let general = b_and(b_not(safe), ne);
    let m1case = b_and(lm1, icmp(L.IntEQ, r, int_min(t)));
    return Ovf.{ val: prod, ovf: b_or(general, m1case) };
}

// base: 0 add, 1 sub, 2 mul
fn op_with_overflow(base: i32, is_signed: bool, l: *mut L.Value, r: *mut L.Value) Ovf {
    let t = L.LLVMTypeOf(l);
    let zero = L.LLVMConstNull(t);
    if base == 0 {
        let val = b_add(l, r);
        if is_signed {
            let xr = b_xor(r, val);  // (right to left, as in the C implementation)
            let x = b_and(b_xor(l, val), xr);
            return Ovf.{ val: val, ovf: icmp(L.IntSLT, x, zero) };
        }
        return Ovf.{ val: val, ovf: icmp(L.IntULT, val, l) };
    }
    if base == 1 {
        let val = b_sub(l, r);
        if is_signed {
            let xv = b_xor(l, val);  // (right to left, as in the C implementation)
            let x = b_and(b_xor(l, r), xv);
            return Ovf.{ val: val, ovf: icmp(L.IntSLT, x, zero) };
        }
        return Ovf.{ val: val, ovf: icmp(L.IntULT, l, r) };
    }
    if L.LLVMGetIntTypeWidth(t) <= 32 {
        return mul_wide_overflow(is_signed, l, r);
    }
    return mul64_overflow(is_signed, l, r);
}

fn saturated(t: *mut L.LType, neg_direction: *mut L.Value) *mut L.Value {
    return sel(neg_direction, int_min(t), int_max(t, true));
}

fn overflow_op(base: i32, is_signed: bool, l: *mut L.Value, r: *mut L.Value) *mut L.Value {
    let t = L.LLVMTypeOf(l);
    let o = op_with_overflow(base, is_signed, l, r);
    if not O.overflow_saturate {
        check_or_trap(b_not(o.ovf), T_OVERFLOW);
        return o.val;
    }
    let zero = L.LLVMConstNull(t);
    let mut s = vnil();
    if base == 2 {
        if is_signed {
            s = saturated(t, icmp(L.IntSLT, b_xor(l, r), zero));
        } else {
            s = int_max(t, false);
        }
    } else if base == 0 {
        if is_signed {
            s = saturated(t, icmp(L.IntSLT, l, zero));
        } else {
            s = int_max(t, false);
        }
    } else {
        if is_signed {
            s = saturated(t, icmp(L.IntSLT, l, zero));
        } else {
            s = zero;
        }
    }
    return sel(o.ovf, s, o.val);
}

// Shift with mathematical semantics for every count (spec §7.6).
fn gen_shift(op: i32, t: *mut ast.Type, x: *mut L.Value, k: *mut L.Value) *mut L.Value {
    let mut lane = t;
    if t.kind == ast.TY_VEC {
        lane = t.elem;
    }
    let n = @int_cast(u32, lane.bits);
    let kty = L.LLVMTypeOf(k);
    let ge = icmp(L.IntUGE, k, ci(kty, @as(u64, n)));
    let lane_ty = L.LLVMIntTypeInContext(ctx, n);
    let arith = (op == lx.TK_SHR) and lane.is_signed;
    let mut fill: u64 = 0;
    if arith {
        fill = @as(u64, n - 1);
    }
    let mut kc = sel(ge, ci(kty, fill), k);
    let kb = L.LLVMGetIntTypeWidth(kty);
    if kb > n {
        kc = trunc(kc, lane_ty);
    } else if kb < n {
        kc = zext(kc, lane_ty);
    }
    if t.kind == ast.TY_VEC {
        let vt = L.LLVMTypeOf(x);
        let v = L.LLVMBuildInsertElement(B, L.LLVMGetUndef(vt), kc, ci32(0), NN());
        let mask = L.LLVMConstNull(L.LLVMVectorType(T_i32, @int_cast(u32, t.lanes)));
        kc = L.LLVMBuildShuffleVector(B, v, L.LLVMGetUndef(vt), mask, NN());
    }
    let mut r = vnil();
    if op == lx.TK_SHL {
        r = L.LLVMBuildShl(B, x, kc, NN());
    } else if arith {
        return L.LLVMBuildAShr(B, x, kc, NN());
    } else {
        r = L.LLVMBuildLShr(B, x, kc, NN());
    }
    return sel(ge, L.LLVMConstNull(L.LLVMTypeOf(x)), r);
}

fn gen_binop(op: i32, t: *mut ast.Type, l: *mut L.Value, r: *mut L.Value) *mut L.Value {
    let is_float = (t.kind == ast.TY_FLOAT) or ty.is_vec_float(t);
    let mut sgn = false;
    if t.kind == ast.TY_INT {
        sgn = t.is_signed;
    } else if t.kind == ast.TY_VEC {
        sgn = t.elem.is_signed;
    }
    if op == lx.TK_PLUS {
        if is_float {
            return L.LLVMBuildFAdd(B, l, r, NN());
        }
        return overflow_op(0, sgn, l, r);
    }
    if op == lx.TK_MINUS {
        if is_float {
            return L.LLVMBuildFSub(B, l, r, NN());
        }
        return overflow_op(1, sgn, l, r);
    }
    if op == lx.TK_STAR {
        if is_float {
            return L.LLVMBuildFMul(B, l, r, NN());
        }
        return overflow_op(2, sgn, l, r);
    }
    if op == lx.TK_SLASH {
        if is_float {
            return L.LLVMBuildFDiv(B, l, r, NN());
        }
        let tt = L.LLVMTypeOf(l);
        check_or_trap(icmp(L.IntNE, r, L.LLVMConstNull(tt)), T_DIVZERO);
        if not sgn {
            return L.LLVMBuildUDiv(B, l, r, NN());
        }
        let rm1 = icmp(L.IntEQ, r, L.LLVMConstAllOnes(tt));  // (right to left, as in the C implementation)
        let ovf = b_and(icmp(L.IntEQ, l, int_min(tt)), rm1);
        if not O.overflow_saturate {
            check_or_trap(b_not(ovf), T_OVERFLOW);
            return L.LLVMBuildSDiv(B, l, r, NN());
        }
        let r2 = sel(ovf, ci(tt, 1), r);
        let q = L.LLVMBuildSDiv(B, l, r2, NN());
        return sel(ovf, int_max(tt, true), q);
    }
    if op == lx.TK_PERCENT {
        let tt = L.LLVMTypeOf(l);
        check_or_trap(icmp(L.IntNE, r, L.LLVMConstNull(tt)), T_DIVZERO);
        if not sgn {
            return L.LLVMBuildURem(B, l, r, NN());
        }
        let m1 = icmp(L.IntEQ, r, L.LLVMConstAllOnes(tt));
        let r2 = sel(m1, ci(tt, 1), r);
        return L.LLVMBuildSRem(B, l, r2, NN());
    }
    if op == lx.TK_PLUS_PCT {
        return b_add(l, r);
    }
    if op == lx.TK_MINUS_PCT {
        return b_sub(l, r);
    }
    if op == lx.TK_STAR_PCT {
        return b_mul(l, r);
    }
    if op == lx.TK_AMP {
        return b_and(l, r);
    }
    if op == lx.TK_PIPE {
        return b_or(l, r);
    }
    if op == lx.TK_CARET {
        return b_xor(l, r);
    }
    if (op == lx.TK_SHL) or (op == lx.TK_SHR) {
        return gen_shift(op, t, l, r);
    }
    // comparisons
    if is_float {
        let mut p = L.RealOGE;
        if op == lx.TK_EQEQ {
            p = L.RealOEQ;
        } else if op == lx.TK_NE {
            p = L.RealUNE;
        } else if op == lx.TK_LT {
            p = L.RealOLT;
        } else if op == lx.TK_LE {
            p = L.RealOLE;
        } else if op == lx.TK_GT {
            p = L.RealOGT;
        }
        return fcmp(p, l, r);
    }
    let mut p = L.IntUGE;
    if sgn {
        p = L.IntSGE;
    }
    if op == lx.TK_EQEQ {
        p = L.IntEQ;
    } else if op == lx.TK_NE {
        p = L.IntNE;
    } else if op == lx.TK_LT {
        p = L.IntULT;
        if sgn {
            p = L.IntSLT;
        }
    } else if op == lx.TK_LE {
        p = L.IntULE;
        if sgn {
            p = L.IntSLE;
        }
    } else if op == lx.TK_GT {
        p = L.IntUGT;
        if sgn {
            p = L.IntSGT;
        }
    }
    return icmp(p, l, r);
}

// ---------------------------------------------------------------------------
// Places
// ---------------------------------------------------------------------------
fn addr_or_spill(e: *mut ast.Expr) *mut L.Value {
    if sema.is_place(e) {
        return gen_addr(e);
    }
    let t = ct(e.ty);
    return spill(t, gen(e));
}

// Is e a place whose storage may be misaligned (reached through a pointer)?
fn via_pointer(e: *mut ast.Expr) bool {
    let k = e.kind;
    if k == ast.E_DEREF {
        return true;
    }
    if k == ast.E_FIELD {
        return (e.self_mode == ast.SELF_DEREF) or ((e.op == ast.FK_STRUCT) and via_pointer(e.a));
    }
    if k == ast.E_INDEX {
        return (e.op != ast.IDX_ARRAY) or via_pointer(e.a);
    }
    return false;
}

fn array_len_value(arr: *mut ast.Type) *mut L.Value {
    return usz(arr.len);
}

fn gen_addr_inner(e: *mut ast.Expr) *mut L.Value {
    let k = e.kind;
    if k == ast.E_IDENT {
        if u.notnil(e.local) {
            return local_ptr(e.local);
        }
        if u.notnil(e.decl) and (e.decl.kind == ast.D_GLOBAL) {
            return global_ref(e.decl);
        }
    } else if k == ast.E_DEREF {
        return gen(e.a);
    } else if k == ast.E_FIELD {
        if (e.op == ast.FK_DECL) and u.notnil(e.decl) and (e.decl.kind == ast.D_GLOBAL) {
            return global_ref(e.decl);
        }
        let at = ct(e.a.ty);
        let mut base = vnil();
        let mut stt = at;
        if e.self_mode == ast.SELF_DEREF {
            base = gen(e.a);
            stt = at.elem;
        } else {
            base = addr_or_spill(e.a);
        }
        return sgep(lty(stt), base, @int_cast(u32, e.index));
    } else if k == ast.E_INDEX {
        let at = ct(e.a.ty);
        if e.op == ast.IDX_SLICE {
            let s = gen(e.a);
            let idx = gen(e.b);
            let ptr = extract(s, 0);
            let len = extract(s, 1);
            check_or_trap(icmp(L.IntULT, idx, len), T_BOUNDS);
            return gep1(lty(at.elem), ptr, idx);
        }
        let mut arr = at;
        let mut base = vnil();
        if e.op == ast.IDX_PTR_ARRAY {
            arr = at.elem;
            base = gen(e.a);
        } else {
            base = addr_or_spill(e.a);
        }
        let idx = gen(e.b);
        check_or_trap(icmp(L.IntULT, idx, array_len_value(arr)), T_BOUNDS);
        return gep2(lty(arr), base, usz(0), idx);
    }
    // not a place: materialize
    let t = ct(e.ty);
    return spill(t, gen(e));
}

// ---------------------------------------------------------------------------
// Calls
// ---------------------------------------------------------------------------
fn emit_call(fty: *mut L.LType, fnv: *mut L.Value, fnty: *mut ast.Type, sret: bool, argv: []mut *mut L.Value,
             argt: []mut *mut ast.Type, callee: []u8) *mut L.Value {
    let argc = argv.len;
    let mut n = argc;
    if sret {
        n += 1;
    }
    let av = u.newn.[*mut L.Value](n + 1);
    let mut k: usize = 0;
    let mut ret_tmp = vnil();
    if sret {
        ret_tmp = temp_alloca(fnty.ret);
        av[k] = ret_tmp;
        k += 1;
    }
    for i in 0..argc {
        if by_ptr(argt[i]) {
            av[k] = spill(argt[i], argv[i]);
        } else {
            av[k] = argv[i];
        }
        k += 1;
    }
    let sf = ss_push(callee);
    let r = call(fty, fnv, av[0..n]);
    ss_pop(sf);
    add_ext_attrs(r, fnty, sret, true);
    if sret {
        return load(fnty.ret, ret_tmp, false);
    }
    if fnty.ret.kind == ast.TY_VOID {
        return vnil();
    }
    return r;
}

fn gen_call(e: *mut ast.Expr) *mut L.Value {
    if e.op == ast.CALLK_VARIANT {
        let ut = ct(e.tval);
        let tmp = temp_alloca(ut);
        let tagp = sgep(lty(ut), tmp, 0);
        let payload = gen(e.args.get(0));
        _ = st(ci(lty(ty.union_tag_type(ut)), @int_cast(u64, e.index)), tagp);
        let pt = ty.type_variant(ut, @int_cast(usize, e.index));
        let pp = sgep(lty(ut), tmp, 1);
        store(pt, payload, pp, true);
        return load(ut, tmp, false);
    }
    if e.op == ast.CALLK_INDIRECT {
        let ft = ct(e.a.ty);
        let fp = gen(e.a);
        let n = e.args.len;
        let av = u.newn.[*mut L.Value](n + 1);
        for i in 0..n {
            av[i] = gen(e.args.get(i));
        }
        let fl = fn_lty(ft);
        return emit_call(fl.t, fp, ft, fl.sret, av[0..n], ft.params, u.nosym());
    }
    let d = e.decl;
    let fi = get_inst(d, subst_gargs(e.gargs));
    let has_recv = u.notnil(e.b);
    let mut n = e.args.len;
    if has_recv {
        n += 1;
    }
    let av = u.newn.[*mut L.Value](n + 1);
    let mut k: usize = 0;
    if has_recv {
        let recv = e.b;
        let sm = e.self_mode;
        if sm == ast.SELF_DEREF {
            let pt = ct(recv.ty);
            av[k] = load(pt.elem, gen(recv), true);
        } else if sm == ast.SELF_ADDR {
            av[k] = gen_addr(recv);
        } else {
            av[k] = gen(recv);
        }
        k += 1;
    }
    for i in 0..e.args.len {
        av[k] = gen(e.args.get(i));
        k += 1;
    }
    let mut argt = fi.fnty.params;
    if d.variadic {  // types of the extra (promoted) arguments
        argt = u.newn.[*mut ast.Type](n + 1);
        let mut off: usize = 0;
        if has_recv {
            off = 1;
        }
        for i in 0..n {
            if i < fi.fnty.params.len {
                argt[i] = fi.fnty.params[i];
            } else {
                argt[i] = ct(e.args.get(i - off).ty);
            }
        }
    }
    return emit_call(fi.llty, fi.fnv, fi.fnty, fi.sret, av[0..n], argt, fi.display);
}

// ---------------------------------------------------------------------------
// catch / try
// ---------------------------------------------------------------------------
fn gen_try(e: *mut ast.Expr) *mut L.Value {
    let eu = ct(e.a.ty);
    let v = gen(e.a);
    let code = extract(v, 0);
    let is_err = icmp(L.IntNE, code, ci(T_i16, 0));
    let err_bb = new_bb("try.err".ptr);
    let ok_bb = new_bb("try.ok".ptr);
    condbr(is_err, err_bb, ok_bb);
    at_end(err_bb);
    etrace_push(code);
    let rv = errunion_value(F.ret, code, vnil());
    emit_defers(0);
    if not terminated() {
        emit_ret(rv);
    }
    at_end(ok_bb);
    if eu.elem.kind == ast.TY_VOID {
        return vnil();
    }
    return from_mem(eu.elem, extract(v, 1));
}

fn gen_catch(e: *mut ast.Expr) *mut L.Value {
    let eu = ct(e.a.ty);
    let vt = eu.elem;
    let v = gen(e.a);
    let code = extract(v, 0);
    let is_err = icmp(L.IntNE, code, ci(T_i16, 0));
    let err_bb = new_bb("catch.err".ptr);
    let ok_bb = new_bb("catch.ok".ptr);
    let merge = new_bb("catch.end".ptr);
    condbr(is_err, err_bb, ok_bb);
    at_end(err_bb);
    etrace_clear();
    if u.notnil(e.capture_local) {
        store(ty.t_error, code, local_ptr(e.capture_local), false);
    }
    let mut hv = vnil();
    let mut hbb = bbnil();
    let hk = e.hkind;
    if hk == ast.H_VALUE {
        hv = gen(e.hexpr);
        if not terminated() {
            hbb = L.LLVMGetInsertBlock(B);
            br(merge);
        }
    } else if hk == ast.H_RETURN {
        gen_return(e.hexpr);
    } else if hk == ast.H_BREAK {
        gen_break(true);
    } else if hk == ast.H_CONTINUE {
        gen_break(false);
    } else if hk == ast.H_BLOCK {
        gen_block(e.hblock);
        if not terminated() {
            br(merge);  // only possible for !void
        }
    }
    if not terminated() {
        _ = L.LLVMBuildUnreachable(B);
    }
    at_end(ok_bb);
    let mut ov = vnil();
    if vt.kind != ast.TY_VOID {
        ov = from_mem(vt, extract(v, 1));
    }
    let obb = L.LLVMGetInsertBlock(B);
    br(merge);
    at_end(merge);
    if vt.kind == ast.TY_VOID {
        return vnil();
    }
    if u.isnil(hbb) or u.isnil(hv) {
        return ov;
    }
    let phi = L.LLVMBuildPhi(B, vty(vt), NN());
    let mut vals: [2]*mut L.Value = [ov, hv];
    let mut bbs: [2]*mut L.BB = [obb, hbb];
    L.LLVMAddIncoming(phi, &mut vals[0], &mut bbs[0], 2);
    return phi;
}

// ---------------------------------------------------------------------------
// Builtins
// ---------------------------------------------------------------------------
fn splat(vt: *mut L.LType, x: *mut L.Value, lanes: u32) *mut L.Value {
    let v = L.LLVMBuildInsertElement(B, L.LLVMGetUndef(vt), x, ci32(0), NN());
    return L.LLVMBuildShuffleVector(B, v, L.LLVMGetUndef(vt), L.LLVMConstNull(L.LLVMVectorType(T_i32, lanes)), NN());
}

fn pairwise_sum(xs: []mut *mut L.Value, is_float: bool) *mut L.Value {
    let n = xs.len;
    if n == 1 {
        return xs[0];
    }
    let h = n / 2;
    let a = pairwise_sum(xs[0..h], is_float);
    let b = pairwise_sum(xs[h..n], is_float);
    if is_float {
        return L.LLVMBuildFAdd(B, a, b, NN());
    }
    return b_add(a, b);
}

fn reduce_add(vt: *mut ast.Type, v: *mut L.Value) *mut L.Value {
    let n = @int_cast(usize, vt.lanes);
    let xs = u.newn.[*mut L.Value](n);
    for i in 0..n {
        xs[i] = L.LLVMBuildExtractElement(B, v, ci32(@as(u64, i)), NN());
    }
    return pairwise_sum(xs, vt.elem.kind == ast.TY_FLOAT);
}

fn ext_int(v: *mut L.Value, to: *mut L.LType, is_signed: bool) *mut L.Value {
    let a = L.LLVMGetIntTypeWidth(L.LLVMTypeOf(v));
    let b = L.LLVMGetIntTypeWidth(to);
    if a == b {
        return v;
    }
    if a > b {
        return trunc(v, to);
    }
    if is_signed {
        return sext(v, to);
    }
    return zext(v, to);
}

fn barg(e: *mut ast.Expr, i: usize) *mut ast.Expr {
    return e.bargs.at(i).e;
}

fn gen_builtin(e: *mut ast.Expr) *mut L.Value {
    let rt = ct(e.ty);
    let id = e.index;
    if id == ast.BI_SIZE_OF {
        return usz(ty.type_size(ct(e.tval)));
    }
    if id == ast.BI_ALIGN_OF {
        return usz(ty.type_align(ct(e.tval)));
    }
    if id == ast.BI_OFFSET_OF {
        return usz(ty.type_field_offset(ct(e.tval), @int_cast(usize, barg(e, 1).index)));
    }
    if (id == ast.BI_MIN_VALUE) or (id == ast.BI_MAX_VALUE) {
        let r = ty.int_range(rt);
        if id == ast.BI_MIN_VALUE {
            return cint(lty(rt), r.lo);
        }
        return cint(lty(rt), r.hi);
    }
    if id == ast.BI_AS {
        let s = ct(barg(e, 1).ty);
        let v = gen(barg(e, 1));
        if s == rt {
            return v;
        }
        if (s.kind == ast.TY_INT) and (rt.kind == ast.TY_INT) {
            return ext_int(v, lty(rt), s.is_signed);
        }
        if (s.kind == ast.TY_FLOAT) and (rt.kind == ast.TY_FLOAT) {
            return L.LLVMBuildFPExt(B, v, lty(rt), NN());
        }
        if (s.kind == ast.TY_INT) and (rt.kind == ast.TY_FLOAT) {
            if s.is_signed {
                return L.LLVMBuildSIToFP(B, v, lty(rt), NN());
            }
            return L.LLVMBuildUIToFP(B, v, lty(rt), NN());
        }
        return v;
    }
    if id == ast.BI_INT_CAST {
        let s = ct(barg(e, 1).ty);
        let v = gen(barg(e, 1));
        if s == rt {
            return v;
        }
        // Range check in the source type's width (no 128-bit arithmetic).
        let stt = L.LLVMTypeOf(v);
        let sb = @int_cast(u32, s.bits);
        let tb = @int_cast(u32, rt.bits);
        let mut ok = ci(T_i1, 1);
        if s.is_signed {
            if not rt.is_signed {
                ok = icmp(L.IntSGE, v, L.LLVMConstNull(stt));
            } else if tb < sb {
                ok = icmp(L.IntSGE, v, cint(stt, u.I128.pow2(tb - 1).neg()));
            }
        }
        let mut tmax_bits = tb;  // T.max = 2^tmax_bits - 1
        if rt.is_signed {
            tmax_bits = tb - 1;
        }
        let mut smax_bits = sb;
        if s.is_signed {
            smax_bits = sb - 1;
        }
        if tmax_bits < smax_bits {
            let hi = cint(stt, u.I128.pow2(tmax_bits).sub(u.I128.of(1)));
            let mut p = L.IntULE;
            if s.is_signed {
                p = L.IntSLE;
            }
            ok = b_and(ok, icmp(p, v, hi));
        }
        check_or_trap(ok, T_INTCAST);
        return ext_int(v, lty(rt), s.is_signed);
    }
    if id == ast.BI_TRUNCATE {
        let s = ct(barg(e, 1).ty);
        return ext_int(gen(barg(e, 1)), lty(rt), s.is_signed);
    }
    if id == ast.BI_INT_FROM_FLOAT {
        let v = gen(barg(e, 1));
        let mut tys: [2]*mut L.LType = [lty(rt), L.LLVMTypeOf(v)];
        let mut args: [1]*mut L.Value = [v];
        return intrinsic(u.pick(rt.is_signed, "llvm.fptosi.sat", "llvm.fptoui.sat"), tys[..], args[..]);
    }
    if id == ast.BI_FLOAT_FROM_INT {
        let s = ct(barg(e, 1).ty);
        let v = gen(barg(e, 1));
        if s.is_signed {
            return L.LLVMBuildSIToFP(B, v, lty(rt), NN());
        }
        return L.LLVMBuildUIToFP(B, v, lty(rt), NN());
    }
    if id == ast.BI_FLOAT_CAST {
        let s = ct(barg(e, 1).ty);
        let v = gen(barg(e, 1));
        if s.bits == rt.bits {
            return v;
        }
        if s.bits > rt.bits {
            return L.LLVMBuildFPTrunc(B, v, lty(rt), NN());
        }
        return L.LLVMBuildFPExt(B, v, lty(rt), NN());
    }
    if id == ast.BI_BIT_CAST {
        let s = ct(barg(e, 1).ty);
        let v = gen(barg(e, 1));
        if s == rt {
            return v;
        }
        if (s.kind == ast.TY_PTR) and (rt.kind != ast.TY_PTR) {
            return ptrtoint(v, lty(rt));
        }
        if (rt.kind == ast.TY_PTR) and (s.kind != ast.TY_PTR) {
            return inttoptr(v, T_ptr);
        }
        if rt.kind == ast.TY_PTR {
            return v;
        }
        return bitcast(v, lty(rt));
    }
    if id == ast.BI_INT_FROM_PTR {
        return ptrtoint(gen(barg(e, 0)), T_isz);
    }
    if id == ast.BI_PTR_FROM_INT {
        return inttoptr(gen(barg(e, 1)), T_ptr);
    }
    if id == ast.BI_PTR_CAST {
        return gen(barg(e, 1));
    }
    if id == ast.BI_SLICE_FROM_PTR {
        let p = gen(barg(e, 0));
        let n = gen(barg(e, 1));
        let s = insert(L.LLVMGetUndef(lty(rt)), p, 0);
        return insert(s, n, 1);
    }
    if id == ast.BI_CAST_SLICE {
        let stt = ct(barg(e, 1).ty);
        let s = gen(barg(e, 1));
        let from = ty.type_size(stt.elem);
        let to = ty.type_size(rt.elem);
        let len = extract(s, 1);
        let bytes = b_mul(len, usz(from));
        let mut nlen = vnil();
        if to == 0 {
            nlen = usz(0);
        } else {
            check_or_trap(icmp(L.IntEQ, L.LLVMBuildURem(B, bytes, usz(to), NN()), usz(0)), T_LENGTH);
            nlen = L.LLVMBuildUDiv(B, bytes, usz(to), NN());
        }
        return insert(s, nlen, 1);
    }
    if id == ast.BI_MEM_COPY {
        let dt = ct(barg(e, 0).ty);
        let d = gen(barg(e, 0));
        let s = gen(barg(e, 1));
        let dl = extract(d, 1);
        let sl = extract(s, 1);
        check_or_trap(icmp(L.IntEQ, dl, sl), T_LENGTH);
        let bytes = b_mul(dl, usz(ty.type_size(dt.elem)));
        let sp0 = extract(s, 0);  // (right to left, as in the C implementation)
        _ = L.LLVMBuildMemMove(B, extract(d, 0), 1, sp0, 1, bytes);
        return vnil();
    }
    if id == ast.BI_MEM_SET {
        let dt = ct(barg(e, 0).ty);
        let d = gen(barg(e, 0));
        let v = gen(barg(e, 1));
        let ptr = extract(d, 0);
        let len = extract(d, 1);
        if (dt.elem.kind == ast.TY_INT) and (dt.elem.bits == 8) {
            _ = L.LLVMBuildMemSet(B, ptr, v, len, 1);
            return vnil();
        }
        // generic loop
        let iv = alloca_in_entry(T_isz, "i".ptr);
        _ = st(usz(0), iv);
        let cond = new_bb("memset.cond".ptr);
        let body = new_bb("memset.body".ptr);
        let end = new_bb("memset.end".ptr);
        br(cond);
        at_end(cond);
        let i = ld(T_isz, iv);
        condbr(icmp(L.IntULT, i, len), body, end);
        at_end(body);
        store(dt.elem, v, gep1(lty(dt.elem), ptr, i), true);
        _ = st(b_add(i, usz(1)), iv);
        br(cond);
        at_end(end);
        return vnil();
    }
    if id == ast.BI_MEM_EQ {
        let stt = ct(barg(e, 0).ty);
        let x = gen(barg(e, 0));
        let y = gen(barg(e, 1));
        let xl = extract(x, 1);
        let yl = extract(y, 1);
        let xp = extract(x, 0);
        let yp = extract(y, 0);
        let bytes = b_mul(xl, usz(ty.type_size(stt.elem)));
        let res = alloca_in_entry(T_i1, "eq".ptr);
        let iv = alloca_in_entry(T_isz, "i".ptr);
        _ = st(icmp(L.IntEQ, xl, yl), res);
        _ = st(usz(0), iv);
        let cond = new_bb("memeq.cond".ptr);
        let body = new_bb("memeq.body".ptr);
        let neq = new_bb("memeq.ne".ptr);
        let end = new_bb("memeq.end".ptr);
        br(cond);
        at_end(cond);
        let i = ld(T_isz, iv);
        let lt = icmp(L.IntULT, i, bytes);  // (right to left, as in the C implementation)
        let more = b_and(ld(T_i1, res), lt);
        condbr(more, body, end);
        at_end(body);
        let bx = ld(T_i8, gep1(T_i8, xp, i));
        let by = ld(T_i8, gep1(T_i8, yp, i));
        _ = st(b_add(i, usz(1)), iv);
        condbr(icmp(L.IntEQ, bx, by), cond, neq);
        at_end(neq);
        _ = st(ci(T_i1, 0), res);
        br(end);
        at_end(end);
        return ld(T_i1, res);
    }
    if id == ast.BI_MEMORY_SIZE {
        return mem_size();
    }
    if id == ast.BI_MEMORY_GROW {
        let prev = mem_grow(gen(barg(e, 0)));
        let failed = icmp(L.IntEQ, prev, L.LLVMConstAllOnes(T_isz));
        let code = sel(failed, errcode_value(sema.error_code_of(u.intern("OutOfMemory"))), ci(T_i16, 0));
        return errunion_value(rt, code, prev);
    }
    if (id == ast.BI_CLZ) or (id == ast.BI_CTZ) {
        let v = gen(barg(e, 0));
        let mut t: [1]*mut L.LType = [L.LLVMTypeOf(v)];
        let mut args: [2]*mut L.Value = [v, ci(T_i1, 0)];
        return intrinsic(u.pick(id == ast.BI_CLZ, "llvm.ctlz", "llvm.cttz"), t[..], args[..]);
    }
    if id == ast.BI_POPCOUNT {
        return intr1("llvm.ctpop", gen(barg(e, 0)));
    }
    if (id == ast.BI_ROTL) or (id == ast.BI_ROTR) {
        let v = gen(barg(e, 0));
        let mut k = gen(barg(e, 1));
        k = ext_int(k, L.LLVMTypeOf(v), false);
        let mut t: [1]*mut L.LType = [L.LLVMTypeOf(v)];
        let mut args: [3]*mut L.Value = [v, v, k];
        return intrinsic(u.pick(id == ast.BI_ROTL, "llvm.fshl", "llvm.fshr"), t[..], args[..]);
    }
    if (id == ast.BI_ADD_SAT) or (id == ast.BI_SUB_SAT) {
        let mut sg = false;
        if rt.kind == ast.TY_INT {
            sg = rt.is_signed;
        } else {
            sg = rt.elem.is_signed;
        }
        let nm = u.f2("llvm.%s%s.sat", u.pick(sg, "s", "u"), u.pick(id == ast.BI_ADD_SAT, "add", "sub"));
        let a1 = gen(barg(e, 1));  // (right to left, as in the C implementation)
        let a0 = gen(barg(e, 0));
        return intr2(nm, a0, a1);
    }
    if id == ast.BI_MUL_SAT {
        let saved = O.overflow_saturate;
        O.overflow_saturate = true;
        let l = gen(barg(e, 0));
        let r = gen(barg(e, 1));
        let v = overflow_op(2, rt.is_signed, l, r);
        O.overflow_saturate = saved;
        return v;
    }
    if (id == ast.BI_ADD_CHECKED) or (id == ast.BI_SUB_CHECKED) or (id == ast.BI_MUL_CHECKED) {
        let t = rt.elem;
        let l = gen(barg(e, 0));
        let r = gen(barg(e, 1));
        let mut base: i32 = 2;
        if id == ast.BI_ADD_CHECKED {
            base = 0;
        } else if id == ast.BI_SUB_CHECKED {
            base = 1;
        }
        let o = op_with_overflow(base, t.is_signed, l, r);
        let code = sel(o.ovf, errcode_value(sema.error_code_of(u.intern("Overflow"))), ci(T_i16, 0));
        return errunion_value(rt, code, o.val);
    }
    if (id == ast.BI_MIN) or (id == ast.BI_MAX) {
        let l = gen(barg(e, 0));
        let r = gen(barg(e, 1));
        let mn = id == ast.BI_MIN;
        if (rt.kind == ast.TY_FLOAT) or ty.is_vec_float(rt) {
            return intr2(u.pick(mn, "llvm.minimum", "llvm.maximum"), l, r);
        }
        let mut sg = false;
        if rt.kind == ast.TY_INT {
            sg = rt.is_signed;
        } else {
            sg = rt.elem.is_signed;
        }
        return intr2(u.f2("llvm.%s%s", u.pick(sg, "s", "u"), u.pick(mn, "min", "max")), l, r);
    }
    if id == ast.BI_ABS {
        let v = gen(barg(e, 0));
        if (rt.kind == ast.TY_FLOAT) or ty.is_vec_float(rt) {
            return intr1("llvm.fabs", v);
        }
        let t = L.LLVMTypeOf(v);
        let mut tys: [1]*mut L.LType = [t];
        if rt.kind == ast.TY_INT {
            let is_min = icmp(L.IntEQ, v, int_min(t));
            if not O.overflow_saturate {
                check_or_trap(b_not(is_min), T_OVERFLOW);
            }
            let mut args: [2]*mut L.Value = [v, ci(T_i1, 0)];
            let mut r = intrinsic("llvm.abs", tys[..], args[..]);
            if O.overflow_saturate {
                r = sel(is_min, int_max(t, true), r);
            }
            return r;
        }
        let mut args: [2]*mut L.Value = [v, ci(T_i1, 0)];
        return intrinsic("llvm.abs", tys[..], args[..]);  // vectors wrap, as in WASM
    }
    if id == ast.BI_SQRT {
        return intr1("llvm.sqrt", gen(barg(e, 0)));
    }
    if id == ast.BI_FLOOR {
        return intr1("llvm.floor", gen(barg(e, 0)));
    }
    if id == ast.BI_CEIL {
        return intr1("llvm.ceil", gen(barg(e, 0)));
    }
    if id == ast.BI_TRUNC {
        return intr1("llvm.trunc", gen(barg(e, 0)));
    }
    if id == ast.BI_NEAREST {
        return intr1("llvm.roundeven", gen(barg(e, 0)));
    }
    if id == ast.BI_FMA {
        let x = gen(barg(e, 0));
        let y = gen(barg(e, 1));
        let z = gen(barg(e, 2));
        let mut t: [1]*mut L.LType = [L.LLVMTypeOf(x)];
        let mut args: [3]*mut L.Value = [x, y, z];
        return intrinsic("llvm.fma", t[..], args[..]);
    }
    if id == ast.BI_REM {
        let x = gen(barg(e, 0));
        let y = gen(barg(e, 1));
        return L.LLVMBuildFRem(B, x, y, NN());
    }
    if id == ast.BI_COPYSIGN {
        let x = gen(barg(e, 0));
        let y = gen(barg(e, 1));
        return intr2("llvm.copysign", x, y);
    }
    if id == ast.BI_SPLAT {
        return splat(lty(rt), gen(barg(e, 0)), @int_cast(u32, rt.lanes));
    }
    if id == ast.BI_VEC {
        let mut v = L.LLVMGetUndef(lty(rt));
        for i in 1..e.bargs.len {
            v = L.LLVMBuildInsertElement(B, v, gen(barg(e, i)), ci32(@as(u64, i - 1)), NN());
        }
        return v;
    }
    if id == ast.BI_EXTRACT_LANE {
        let v = gen(barg(e, 0));
        return L.LLVMBuildExtractElement(B, v, ci32(barg(e, 1).cval.lo), NN());
    }
    if id == ast.BI_REPLACE_LANE {
        let v = gen(barg(e, 0));
        let x = gen(barg(e, 2));
        return L.LLVMBuildInsertElement(B, v, x, ci32(barg(e, 1).cval.lo), NN());
    }
    if id == ast.BI_LOAD {
        let stt = ct(barg(e, 1).ty);
        let s = gen(barg(e, 1));
        let len = extract(s, 1);
        let bytes = b_mul(len, usz(ty.type_size(stt.elem)));
        check_or_trap(icmp(L.IntUGE, bytes, usz(16)), T_LENGTH);
        return load(rt, extract(s, 0), true);
    }
    if id == ast.BI_STORE {
        let stt = ct(barg(e, 0).ty);
        let vt = ct(barg(e, 1).ty);
        let s = gen(barg(e, 0));
        let v = gen(barg(e, 1));
        let len = extract(s, 1);
        let bytes = b_mul(len, usz(ty.type_size(stt.elem)));
        check_or_trap(icmp(L.IntUGE, bytes, usz(16)), T_LENGTH);
        store(vt, v, extract(s, 0), true);
        return vnil();
    }
    if id == ast.BI_SHUFFLE {
        let x = gen(barg(e, 0));
        let y = gen(barg(e, 1));
        let idx = barg(e, 2);
        let m = u.newn.[*mut L.Value](idx.args.len + 1);
        for i in 0..idx.args.len {
            m[i] = ci32(idx.args.get(i).cval.lo);
        }
        return L.LLVMBuildShuffleVector(B, x, y, L.LLVMConstVector(&mut m[0], @int_cast(u32, idx.args.len)), NN());
    }
    if id == ast.BI_SWIZZLE {
        let x = gen(barg(e, 0));
        let idx = gen(barg(e, 1));
        let mut r = L.LLVMConstNull(lty(rt));
        for i in @as(u64, 0)..16 {
            let li = ci32(i);
            let k = L.LLVMBuildExtractElement(B, idx, li, NN());
            let inb = icmp(L.IntULT, k, ci(T_i8, 16));
            let kk = b_and(k, ci(T_i8, 15));
            let mut el = L.LLVMBuildExtractElement(B, x, zext(kk, T_i32), NN());
            el = sel(inb, el, ci(T_i8, 0));
            r = L.LLVMBuildInsertElement(B, r, el, li, NN());
        }
        return r;
    }
    if (id >= ast.BI_LANES_EQ) and (id <= ast.BI_LANES_GE) {
        let vt = ct(barg(e, 0).ty);
        let x = gen(barg(e, 0));
        let y = gen(barg(e, 1));
        let ops = [lx.TK_EQEQ, lx.TK_NE, lx.TK_LT, lx.TK_LE, lx.TK_GT, lx.TK_GE];
        let c = gen_binop(ops[@int_cast(usize, id - ast.BI_LANES_EQ)], vt, x, y);
        return sext(c, lty(rt));
    }
    if id == ast.BI_SELECT {
        let m = gen(barg(e, 0));
        let x = gen(barg(e, 1));
        let y = gen(barg(e, 2));
        let mt = L.LLVMTypeOf(m);
        let xm = bitcast(x, mt);
        let ym = bitcast(y, mt);
        let ynm = b_and(ym, b_not(m));  // (right to left, as in the C implementation)
        let r = b_or(b_and(xm, m), ynm);
        return bitcast(r, lty(rt));
    }
    if id == ast.BI_ANY_TRUE {
        let v = gen(barg(e, 0));
        let bits = bitcast(v, T_i128);
        return icmp(L.IntNE, bits, L.LLVMConstNull(T_i128));
    }
    if id == ast.BI_ALL_TRUE {
        let v = gen(barg(e, 0));
        let vt = ct(barg(e, 0).ty);
        let mut iv = v;
        if vt.elem.kind == ast.TY_FLOAT {
            iv = bitcast(v, L.LLVMVectorType(tint(vt.elem.bits), @int_cast(u32, vt.lanes)));
        }
        let nz = icmp(L.IntNE, iv, L.LLVMConstNull(L.LLVMTypeOf(iv)));
        return intr1("llvm.vector.reduce.and", nz);
    }
    if id == ast.BI_BITMASK {
        let vt = ct(barg(e, 0).ty);
        let v = gen(barg(e, 0));
        let neg = icmp(L.IntSLT, v, L.LLVMConstNull(L.LLVMTypeOf(v)));
        let bits = bitcast(neg, tint(vt.lanes));
        return zext(bits, T_i32);
    }
    if id == ast.BI_REDUCE_ADD {
        return reduce_add(ct(barg(e, 0).ty), gen(barg(e, 0)));
    }
    if id == ast.BI_DOT {
        let vt = ct(barg(e, 0).ty);
        let x = gen(barg(e, 0));
        let y = gen(barg(e, 1));
        if vt.elem.kind == ast.TY_FLOAT {
            return reduce_add(vt, L.LLVMBuildFMul(B, x, y, NN()));
        }
        let w = L.LLVMVectorType(T_i32, 8);
        let sy = sext(y, w);  // (right to left, as in the C implementation)
        let p = b_mul(sext(x, w), sy);
        let mut ev: [4]*mut L.Value = [ci32(0), ci32(2), ci32(4), ci32(6)];
        let mut od: [4]*mut L.Value = [ci32(1), ci32(3), ci32(5), ci32(7)];
        let pe = L.LLVMBuildShuffleVector(B, p, L.LLVMGetUndef(w), L.LLVMConstVector(&mut ev[0], 4), NN());
        let po = L.LLVMBuildShuffleVector(B, p, L.LLVMGetUndef(w), L.LLVMConstVector(&mut od[0], 4), NN());
        return b_add(pe, po);
    }
    if id == ast.BI_TRAP {
        emit_trap(T_TRAP);
        return vnil();
    }
    if id == ast.BI_EXPECT {
        check_or_trap(gen(barg(e, 0)), T_EXPECT);
        return vnil();
    }
    if id == ast.BI_EXPECT_EQ {
        gen_expect_eq(e);
        return vnil();
    }
    if id == ast.BI_FENCE {
        _ = L.LLVMBuildFence(B, L.SeqCst, 0, NN());
        return vnil();
    }
    if (id >= ast.BI_ATOMIC_LOAD) and (id <= ast.BI_ATOMIC_NOTIFY) {
        return gen_atomic(e);
    }
    if id == ast.BI_ERROR_NAME {
        let code = gen(barg(e, 0));
        let f = error_name_fn();
        let mut args: [1]*mut L.Value = [code];
        return call(error_name_fty(), f, args[..]);
    }
    internal_error(u.f1("builtin @%s not implemented", e.name));
    return vnil();
}

// Atomics (spec §12.8): all sequentially consistent, naturally aligned.
// A misaligned atomic access traps (as WebAssembly requires); on native
// targets the check is explicit so behavior matches.
fn native_rt_fn(name: []u8, fty: *mut L.LType) *mut L.Value {
    let mut f = L.LLVMGetNamedFunction(M, u.cz(name));
    if u.isnil(f) {
        f = L.LLVMAddFunction(M, u.cz(name), fty);
    }
    g_needs_native_runtime = true;
    return f;
}

// @expect_eq(a, b): on failure, record both values (and how to show them) in
// phalange.trap_info before trapping. kind: 1 signed, 2 unsigned, 3 float,
// 4 bool, 5 bytes ([]u8), 6 error, 7 address.
fn store_info(slot: u64, v: *mut L.Value) void {
    _ = st(v, trap_info_slot(slot));
}

fn gen_expect_eq(e: *mut ast.Expr) void {
    let xa = barg(e, 0);
    let xb = barg(e, 1);
    let t = ct(xa.ty);
    let a = gen(xa);
    let b = gen(xb);
    let mut eq = vnil();
    let mut kind: u64 = 0;
    let mut ia = vnil();
    let mut ib = vnil();
    let mut la = vnil();
    let mut lb = vnil();
    if t.kind == ast.TY_FLOAT {
        eq = fcmp(L.RealOEQ, a, b);
        kind = 3;
        if t.bits == 32 {
            ia = bitcast(L.LLVMBuildFPExt(B, a, T_f64, NN()), T_i64);
            ib = bitcast(L.LLVMBuildFPExt(B, b, T_f64, NN()), T_i64);
        } else {
            ia = bitcast(a, T_i64);
            ib = bitcast(b, T_i64);
        }
    } else if t.kind == ast.TY_SLICE {
        let pa = extract(a, 0);
        let pb = extract(b, 0);
        la = extract(a, 1);
        lb = extract(b, 1);
        // equal lengths and bytes
        let res = alloca_in_entry(T_i1, "eq".ptr);
        let iv = alloca_in_entry(T_isz, "i".ptr);
        _ = st(icmp(L.IntEQ, la, lb), res);
        _ = st(usz(0), iv);
        let cond = new_bb("eq.cond".ptr);
        let body = new_bb("eq.body".ptr);
        let neq = new_bb("eq.ne".ptr);
        let end = new_bb("eq.end".ptr);
        br(cond);
        at_end(cond);
        let i = ld(T_isz, iv);
        let lt = icmp(L.IntULT, i, la);  // (right to left, as in the C implementation)
        let more = b_and(ld(T_i1, res), lt);
        condbr(more, body, end);
        at_end(body);
        let bx = ld(T_i8, gep1(T_i8, pa, i));
        let by = ld(T_i8, gep1(T_i8, pb, i));
        _ = st(b_add(i, usz(1)), iv);
        condbr(icmp(L.IntEQ, bx, by), cond, neq);
        at_end(neq);
        _ = st(ci(T_i1, 0), res);
        br(end);
        at_end(end);
        eq = ld(T_i1, res);
        kind = 5;
        ia = zext(ptrtoint(pa, T_isz), T_i64);
        ib = zext(ptrtoint(pb, T_isz), T_i64);
        la = zext(la, T_i64);
        lb = zext(lb, T_i64);
    } else if (t.kind == ast.TY_PTR) or (t.kind == ast.TY_FN) {
        eq = icmp(L.IntEQ, a, b);
        kind = 7;
        ia = zext(ptrtoint(a, T_isz), T_i64);
        ib = zext(ptrtoint(b, T_isz), T_i64);
    } else {  // integers, bool, error
        eq = icmp(L.IntEQ, a, b);
        let sg = (t.kind == ast.TY_INT) and t.is_signed;
        if t.kind == ast.TY_BOOL {
            kind = 4;
        } else if t.kind == ast.TY_ERROR {
            kind = 6;
        } else if sg {
            kind = 1;
        } else {
            kind = 2;
        }
        if L.LLVMGetIntTypeWidth(L.LLVMTypeOf(a)) == 64 {
            ia = a;
        } else if sg {
            ia = sext(a, T_i64);
        } else {
            ia = zext(a, T_i64);
        }
        if L.LLVMGetIntTypeWidth(L.LLVMTypeOf(b)) == 64 {
            ib = b;
        } else if sg {
            ib = sext(b, T_i64);
        } else {
            ib = zext(b, T_i64);
        }
    }
    let ok = new_bb("expect.ok".ptr);
    let fail = new_bb("expect.fail".ptr);
    condbr(eq, ok, fail);
    at_end(fail);
    store_info(1, ci(T_i64, kind));
    store_info(2, ia);
    store_info(3, ib);
    if u.notnil(la) {
        store_info(4, la);
    } else {
        store_info(4, ci(T_i64, 0));
    }
    if u.notnil(lb) {
        store_info(5, lb);
    } else {
        store_info(5, ci(T_i64, 0));
    }
    call_trap_site(site_record(T_EXPECT, e.span));
    _ = L.LLVMBuildUnreachable(B);
    at_end(ok);
}

fn gen_atomic(e: *mut ast.Expr) *mut L.Value {
    let id = e.index;
    let pt = ct(barg(e, 0).ty);
    let t = pt.elem;
    let p = gen(barg(e, 0));
    let size = ty.type_size(t);
    let asz = @int_cast(u32, size);
    if size > 1 {
        let addr = ptrtoint(p, T_isz);
        check_or_trap(icmp(L.IntEQ, b_and(addr, usz(size - 1)), usz(0)), T_ATOMIC_ALIGN);
    }
    let sc = L.SeqCst;
    if id == ast.BI_ATOMIC_LOAD {
        let v = ld(lty(t), p);
        L.LLVMSetOrdering(v, sc);
        L.LLVMSetAlignment(v, asz);
        return v;
    }
    if id == ast.BI_ATOMIC_STORE {
        let v = gen(barg(e, 1));
        let s = st(v, p);
        L.LLVMSetOrdering(s, sc);
        L.LLVMSetAlignment(s, asz);
        return vnil();
    }
    if id == ast.BI_ATOMIC_CAS {
        let expv = gen(barg(e, 1));
        let newv = gen(barg(e, 2));
        let r = L.LLVMBuildAtomicCmpXchg(B, p, expv, newv, sc, sc, 0);
        L.LLVMSetAlignment(r, asz);
        return extract(r, 0);
    }
    if id == ast.BI_ATOMIC_WAIT {
        let expv = gen(barg(e, 1));
        let timeout = gen(barg(e, 2));
        let mut args: [3]*mut L.Value = [p, expv, timeout];
        if ty.g_target.is_wasm {
            return intrinsic("llvm.wasm.memory.atomic.wait32", no_tys(), args[..]);
        }
        let mut ps: [3]*mut L.LType = [T_ptr, T_i32, T_i64];
        let fty = fn_type(T_i32, ps[..], false);
        let f = native_rt_fn("ph_wait32", fty);
        return call(fty, f, args[..]);
    }
    if id == ast.BI_ATOMIC_NOTIFY {
        let count = gen(barg(e, 1));
        let mut args: [2]*mut L.Value = [p, count];
        if ty.g_target.is_wasm {
            return intrinsic("llvm.wasm.memory.atomic.notify", no_tys(), args[..]);
        }
        let mut ps: [2]*mut L.LType = [T_ptr, T_i32];
        let fty = fn_type(T_i32, ps[..], false);
        let f = native_rt_fn("ph_notify32", fty);
        return call(fty, f, args[..]);
    }
    let sg = t.is_signed;
    let mut op = L.RMWMax;
    if not sg {
        op = L.RMWUMax;
    }
    if id == ast.BI_ATOMIC_ADD {
        op = L.RMWAdd;
    } else if id == ast.BI_ATOMIC_SUB {
        op = L.RMWSub;
    } else if id == ast.BI_ATOMIC_AND {
        op = L.RMWAnd;
    } else if id == ast.BI_ATOMIC_OR {
        op = L.RMWOr;
    } else if id == ast.BI_ATOMIC_XOR {
        op = L.RMWXor;
    } else if id == ast.BI_ATOMIC_XCHG {
        op = L.RMWXchg;
    } else if id == ast.BI_ATOMIC_MIN {
        op = L.RMWMin;
        if not sg {
            op = L.RMWUMin;
        }
    }
    let v = gen(barg(e, 1));
    let r = L.LLVMBuildAtomicRMW(B, op, p, v, sc, 0);
    L.LLVMSetAlignment(r, asz);
    return r;
}

// Entry point for threads on wasm: a new thread is a new instance sharing
// the memory; the host calls this export with the thread function, its
// argument and the top of a stack allocated for the thread. The function is
// frameless, so it touches no stack memory before switching stacks.
fn emit_thread_start() void {
    let mut ps: [3]*mut L.LType = [T_ptr, T_ptr, T_ptr];
    let fty = fn_type(T_void, ps[..], false);
    let f = L.LLVMAddFunction(M, "phalange_thread_start".ptr, fty);
    add_str_attr(f, "wasm-export-name", "phalange_thread_start");
    g_export_names.push(u.intern("phalange_thread_start"));
    let bb = L.LLVMAppendBasicBlockInContext(ctx, f, "entry".ptr);
    let saved = L.LLVMGetInsertBlock(B);
    at_end(bb);
    let mut top = L.LLVMGetParam(f, 2);
    if stack_traces_on() {
        // The thread's shadow stack sits at the top of its stack.
        let t = ss_type();
        let size = (L.LLVMABISizeOfType(TD, t) + 15) & ~@as(u64, 15);
        let neg = L.LLVMConstInt(T_isz, 0 -% size, 1);
        let base = gep1(T_i8, top, neg);
        _ = L.LLVMBuildMemSet(B, base, ci(T_i8, 0), usz(@bit_cast(i64, size)), 16);
        ss_globals();
        _ = st(base, ss_global);
        top = base;
    }
    let mut tys: [1]*mut L.LType = [T_ptr];
    let mut targs: [1]*mut L.Value = [top];
    _ = intrinsic("llvm.stackrestore", tys[..], targs[..]);
    let mut eps: [1]*mut L.LType = [T_ptr];
    let ety = fn_type(T_void, eps[..], false);
    let mut args: [1]*mut L.Value = [L.LLVMGetParam(f, 1)];
    _ = call(ety, L.LLVMGetParam(f, 0), args[..]);
    _ = L.LLVMBuildRetVoid(B);
    if u.notnil(saved) {
        at_end(saved);
    }
}

let mut g_errtab: *mut L.Value = @ptr_from_int(*mut L.Value, 0);

fn error_name_table() *mut L.Value {
    if u.notnil(g_errtab) {
        return g_errtab;
    }
    let names = sema.g_prog.error_names;
    let n = names.len;
    let vs = u.newn.[*mut L.Value](n + 1);
    vs[0] = const_slice(string_ptr("<unknown>"), 9);
    for i in 0..n {
        let s = names.get(i);
        vs[i + 1] = const_slice(string_ptr(s), @int_cast(i64, s.len));
    }
    let stt = lty(ty.type_slice(ty.t_u8, false));
    g_errtab = L.LLVMAddGlobal(M, L.LLVMArrayType2(stt, @as(u64, n) + 1), "phalange.error_names".ptr);
    L.LLVMSetInitializer(g_errtab, L.LLVMConstArray2(stt, &mut vs[0], @as(u64, n) + 1));
    L.LLVMSetLinkage(g_errtab, L.PrivateLinkage);
    L.LLVMSetGlobalConstant(g_errtab, 1);
    return g_errtab;
}

fn error_name_fty() *mut L.LType {
    let stt = lty(ty.type_slice(ty.t_u8, false));
    let mut ps: [1]*mut L.LType = [T_i16];
    return fn_type(stt, ps[..], false);
}

// phalange.error_name(code) []u8: the name table lives with the error codes
// (in the link unit when units are compiled separately).
fn error_name_fn() *mut L.Value {
    let stt = lty(ty.type_slice(ty.t_u8, false));
    let fty = error_name_fty();
    let existing = L.LLVMGetNamedFunction(M, "phalange.error_name".ptr);
    if u.notnil(existing) {
        return existing;
    }
    let f = L.LLVMAddFunction(M, "phalange.error_name".ptr, fty);
    if O.unit == CG_MODULE {
        return f;
    }
    if O.unit == CG_WHOLE {
        L.LLVMSetLinkage(f, L.InternalLinkage);
    }
    let saved = L.LLVMGetInsertBlock(B);
    at_end(L.LLVMAppendBasicBlockInContext(ctx, f, "entry".ptr));
    let tab = error_name_table();
    let n = sema.g_prog.error_names.len;
    let mut c = zext(L.LLVMGetParam(f, 0), T_isz);
    c = sel(icmp(L.IntULE, c, usz(@int_cast(i64, n))), c, usz(0));
    _ = L.LLVMBuildRet(B, ld(stt, gep2(L.LLVMArrayType2(stt, @as(u64, n) + 1), tab, usz(0), c)));
    if u.notnil(saved) {
        at_end(saved);
    }
    return f;
}

// Linear memory growth. On wasm these are the real instructions; on native
// targets a reserved, 64 KiB-aligned region emulates linear memory pages.
const NATIVE_HEAP_PAGES: u64 = 4096;
let mut native_heap: *mut L.Value = @ptr_from_int(*mut L.Value, 0);
let mut native_pages: *mut L.Value = @ptr_from_int(*mut L.Value, 0);

fn ensure_native_heap() void {
    if u.notnil(native_heap) {
        return;
    }
    let ht = L.LLVMArrayType2(T_i8, NATIVE_HEAP_PAGES * 65536);
    native_heap = shared_global(ht, "phalange.heap", L.LLVMConstNull(ht), 0);
    L.LLVMSetAlignment(native_heap, 65536);
    native_pages = shared_global(T_isz, "phalange.heap_pages", usz(0), 0);
}

fn mem_size() *mut L.Value {
    if ty.g_target.is_wasm {
        let mut t: [1]*mut L.LType = [T_isz];
        let mut args: [1]*mut L.Value = [ci32(0)];
        return intrinsic("llvm.wasm.memory.size", t[..], args[..]);
    }
    ensure_native_heap();
    let base = L.LLVMBuildUDiv(B, ptrtoint(native_heap, T_isz), usz(65536), NN());
    return b_add(base, ld(T_isz, native_pages));
}

fn mem_grow(pages: *mut L.Value) *mut L.Value {
    if ty.g_target.is_wasm {
        let mut t: [1]*mut L.LType = [T_isz];
        let mut args: [2]*mut L.Value = [ci32(0), pages];
        return intrinsic("llvm.wasm.memory.grow", t[..], args[..]);
    }
    ensure_native_heap();
    let used = ld(T_isz, native_pages);
    let nu = b_add(used, pages);
    let ge = icmp(L.IntUGE, nu, used);  // (right to left, as in the C implementation)
    let ok = b_and(icmp(L.IntULE, nu, usz(@bit_cast(i64, NATIVE_HEAP_PAGES))), ge);
    _ = st(sel(ok, nu, used), native_pages);
    let base = L.LLVMBuildUDiv(B, ptrtoint(native_heap, T_isz), usz(65536), NN());
    return sel(ok, b_add(base, used), L.LLVMConstAllOnes(T_isz));
}

// ---------------------------------------------------------------------------
// Expressions
// ---------------------------------------------------------------------------
fn gen_struct_lit(e: *mut ast.Expr) *mut L.Value {
    let t = ct(e.ty);
    let mut v = L.LLVMGetUndef(lty(t));
    for i in 0..e.inits.len {
        let ft = ty.type_field(t, i);
        let fv = gen(e.inits.at(i).value);
        v = insert(v, to_mem(ft, fv), @int_cast(u32, i));
    }
    return v;
}

fn gen_array_lit(e: *mut ast.Expr) *mut L.Value {
    let t = ct(e.ty);
    if e.kind == ast.E_ARRAY_LIT {
        let mut v = L.LLVMGetUndef(lty(t));
        for i in 0..e.args.len {
            v = insert(v, to_mem(t.elem, gen(e.args.get(i))), @int_cast(u32, i));
        }
        return v;
    }
    let x = gen(e.a);
    if (L.LLVMIsConstant(x) != 0) and (L.LLVMIsNull(x) != 0) {
        return L.LLVMConstNull(lty(t));
    }
    if t.len <= 16 {
        let mut v = L.LLVMGetUndef(lty(t));
        for i in 0..@int_cast(u32, t.len) {
            v = insert(v, to_mem(t.elem, x), i);
        }
        return v;
    }
    let tmp = temp_alloca(t);
    let iv = alloca_in_entry(T_isz, "i".ptr);
    _ = st(usz(0), iv);
    let cond = new_bb("fill.cond".ptr);
    let body = new_bb("fill.body".ptr);
    let end = new_bb("fill.end".ptr);
    br(cond);
    at_end(cond);
    let i = ld(T_isz, iv);
    condbr(icmp(L.IntULT, i, usz(t.len)), body, end);
    at_end(body);
    store(t.elem, x, gep2(lty(t), tmp, usz(0), i), false);
    _ = st(b_add(i, usz(1)), iv);
    br(cond);
    at_end(end);
    return load(t, tmp, false);
}

fn gen_unary(e: *mut ast.Expr) *mut L.Value {
    let t = ct(e.ty);
    let op = e.op;
    if (op == lx.KW_NOT) or (op == lx.TK_TILDE) {
        return b_not(gen(e.a));
    }
    if op == lx.TK_AMP {
        return gen_addr(e.a);
    }
    if op == lx.TK_MINUS_PCT {
        return L.LLVMBuildNeg(B, gen(e.a), NN());
    }
    if op == lx.TK_MINUS {
        let v = gen(e.a);
        if (t.kind == ast.TY_FLOAT) or ty.is_vec_float(t) {
            return L.LLVMBuildFNeg(B, v, NN());
        }
        return overflow_op(1, true, L.LLVMConstNull(L.LLVMTypeOf(v)), v);
    }
    internal_error("bad unary");
    return vnil();
}

fn gen_binary(e: *mut ast.Expr) *mut L.Value {
    if (e.op == lx.KW_AND) or (e.op == lx.KW_OR) {
        let is_and = e.op == lx.KW_AND;
        let l = gen(e.a);
        let lbb = L.LLVMGetInsertBlock(B);
        let rhs = new_bb(u.pick(is_and, "and.rhs", "or.rhs").ptr);
        let end = new_bb(u.pick(is_and, "and.end", "or.end").ptr);
        if is_and {
            condbr(l, rhs, end);
        } else {
            condbr(l, end, rhs);
        }
        at_end(rhs);
        let r = gen(e.b);
        let rbb = L.LLVMGetInsertBlock(B);
        br(end);
        at_end(end);
        let phi = L.LLVMBuildPhi(B, T_i1, NN());
        let mut sc: u64 = 1;
        if is_and {
            sc = 0;
        }
        let mut vals: [2]*mut L.Value = [ci(T_i1, sc), r];
        let mut bbs: [2]*mut L.BB = [lbb, rbb];
        L.LLVMAddIncoming(phi, &mut vals[0], &mut bbs[0], 2);
        return phi;
    }
    let lt = ct(e.a.ty);
    let l = gen(e.a);
    let r = gen(e.b);
    return gen_binop(e.op, lt, l, r);
}

fn gen_slice(e: *mut ast.Expr) *mut L.Value {
    let at = ct(e.a.ty);
    let rt = ct(e.ty);
    let mut ptr = vnil();
    let mut len = vnil();
    if e.op == ast.IDX_SLICE {
        let s = gen(e.a);
        ptr = extract(s, 0);
        len = extract(s, 1);
    } else {
        let mut arr = at;
        if e.op == ast.IDX_PTR_ARRAY {
            arr = at.elem;
            ptr = gen(e.a);
        } else {
            ptr = gen_addr(e.a);
        }
        len = array_len_value(arr);
    }
    let mut lo = usz(0);
    if u.notnil(e.b) {
        lo = gen(e.b);
    }
    let mut hi = len;
    if u.notnil(e.c) {
        hi = gen(e.c);
    }
    let hl = icmp(L.IntULE, hi, len);  // (right to left, as in the C implementation)
    let ok = b_and(icmp(L.IntULE, lo, hi), hl);
    check_or_trap(ok, T_BOUNDS);
    let np = gep1(lty(rt.elem), ptr, lo);
    let s = insert(L.LLVMGetUndef(lty(rt)), np, 0);
    return insert(s, b_sub(hi, lo), 1);
}

fn gen_inner(e: *mut ast.Expr) *mut L.Value {
    let t = ct(e.ty);
    if u.notnil(t) and (t.kind == ast.TY_NEVER) {
        // The expression diverges; generate it and hand back a placeholder.
        if e.kind == ast.E_BUILTIN {
            _ = gen_builtin(e);
        }
        let mut want = tnil();
        if u.notnil(e.tval) {
            want = ct(e.tval);
        }
        if u.notnil(want) and (want.kind != ast.TY_VOID) and (want.kind != ast.TY_NEVER) {
            return L.LLVMGetPoison(vty(want));
        }
        return vnil();
    }
    if e.is_const and u.notnil(t) {
        if t.kind == ast.TY_INT {
            return cint(lty(t), e.cval);
        }
        if t.kind == ast.TY_FLOAT {
            return L.LLVMConstReal(lty(t), e.fval);
        }
        if t.kind == ast.TY_BOOL {
            let mut bv: u64 = 0;
            if e.bval {
                bv = 1;
            }
            return ci(T_i1, bv);
        }
    }
    let k = e.kind;
    if k == ast.E_IDENT {
        if u.notnil(e.local) {
            return load(t, local_ptr(e.local), false);
        }
        if u.notnil(e.tval) and (e.tval.kind == ast.TY_PARAM) and e.tval.is_const {
            let c = ct(e.tval);
            return cint(lty(t), u.I128.of(c.cval));
        }
        if u.notnil(e.decl) {
            let dk = e.decl.kind;
            if dk == ast.D_GLOBAL {
                return load(t, global_ref(e.decl), false);
            }
            if (dk == ast.D_FN) or (dk == ast.D_HOST_FN) {
                return get_inst(e.decl, subst_gargs(e.gargs)).fnv;
            }
            if dk == ast.D_CONST {
                if (O.unit == CG_MODULE) and type_mentions_error(t) {
                    return gen(e.decl.cexpr);
                }
                return gen_const(e.decl.cexpr, t);
            }
        }
    } else if k == ast.E_STRING {
        return const_slice(string_ptr(e.str), @int_cast(i64, e.str.len));
    } else if k == ast.E_BOOL {
        let mut bv: u64 = 0;
        if e.bval {
            bv = 1;
        }
        return ci(T_i1, bv);
    } else if k == ast.E_INT {
        return cint(lty(t), u.I128.ofu(e.ival));
    } else if k == ast.E_FLOAT {
        return L.LLVMConstReal(lty(t), e.fval);
    } else if k == ast.E_STRUCT_LIT {
        return gen_struct_lit(e);
    } else if (k == ast.E_ARRAY_LIT) or (k == ast.E_ARRAY_REPEAT) {
        return gen_array_lit(e);
    } else if k == ast.E_ERROR_VAL {
        return errcode_value(@truncate(u32, e.cval.lo));
    } else if k == ast.E_BUILTIN {
        return gen_builtin(e);
    } else if k == ast.E_WASM {
        return gen_wasm(&mut e.wins, t);
    } else if k == ast.E_UNARY {
        return gen_unary(e);
    } else if k == ast.E_BINARY {
        return gen_binary(e);
    } else if k == ast.E_CATCH {
        return gen_catch(e);
    } else if k == ast.E_TRY {
        return gen_try(e);
    } else if k == ast.E_CALL {
        return gen_call(e);
    } else if k == ast.E_FIELD {
        let fk = e.op;
        if fk == ast.FK_STRUCT {
            if sema.is_place(e) {
                return load(t, gen_addr(e), via_pointer(e));
            }
            if e.self_mode == ast.SELF_DEREF {
                return load(t, gen_addr(e), true);
            }
            return from_mem(t, extract(gen(e.a), @int_cast(u32, e.index)));
        }
        if fk == ast.FK_SLICE_LEN {
            return extract(gen(e.a), 1);
        }
        if fk == ast.FK_SLICE_PTR {
            return extract(gen(e.a), 0);
        }
        if fk == ast.FK_ARRAY_LEN {
            return usz(ct(e.a.ty).len);
        }
        if fk == ast.FK_VARIANT_UNIT {
            let tmp = temp_alloca(t);
            _ = st(ci(lty(ty.union_tag_type(t)), @int_cast(u64, e.index)), sgep(lty(t), tmp, 0));
            return load(t, tmp, false);
        }
        if fk == ast.FK_CONST {
            return gen_const(e.decl.cexpr, t);
        }
        if fk == ast.FK_DECL {
            if e.decl.kind == ast.D_GLOBAL {
                return load(t, global_ref(e.decl), false);
            }
            return get_inst(e.decl, subst_gargs(e.gargs)).fnv;
        }
    } else if k == ast.E_DEREF {
        return load(t, gen(e.a), true);
    } else if k == ast.E_INDEX {
        return load(t, gen_addr(e), via_pointer(e) or (e.op != ast.IDX_ARRAY));
    } else if k == ast.E_SLICE {
        return gen_slice(e);
    } else if k == ast.E_APPLY {
        return get_inst(e.decl, subst_gargs(e.gargs)).fnv;
    } else if k == ast.E_CONV {
        if e.op == ast.CONV_NOOP {
            return gen(e.a);
        }
        if e.op == ast.CONV_WRAP_OK {
            let mut v = vnil();
            if t.elem.kind != ast.TY_VOID {
                v = gen(e.a);
            }
            return errunion_value(t, ci(T_i16, 0), v);
        }
        if e.op == ast.CONV_WRAP_ERR {
            return errunion_value(t, gen(e.a), vnil());
        }
    }
    internal_error(u.f1("cannot generate expression kind %s", u.idec(e.kind)));
    return vnil();
}

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------
fn gen_block(b: *mut ast.Stmt) void {
    let height = F.defers.len;
    for i in 0..b.stmts.len {
        if terminated() {
            break;
        }
        gen_stmt(b.stmts.get(i));
    }
    if not terminated() {
        emit_defers(height);
    }
    F.defers.len = height;
}

fn gen_if(s: *mut ast.Stmt) void {
    let c = gen(s.e);
    let then_bb = new_bb("if.then".ptr);
    let end = new_bb("if.end".ptr);
    let mut else_bb = end;
    if u.notnil(s.els) {
        else_bb = new_bb("if.else".ptr);
    }
    condbr(c, then_bb, else_bb);
    at_end(then_bb);
    gen_block(s.body);
    if not terminated() {
        br(end);
    }
    if u.notnil(s.els) {
        at_end(else_bb);
        if s.els.kind == ast.S_IF {
            gen_if(s.els);
        } else {
            gen_block(s.els);
        }
        if not terminated() {
            br(end);
        }
    }
    at_end(end);
}

fn push_loop(brk: *mut L.BB, cont: *mut L.BB) void {
    F.loops.push(LoopCtx.{ brk: brk, cont: cont, defer_height: F.defers.len });
}

fn gen_for(s: *mut ast.Stmt) void {
    let vt = ct(s.local.ty);
    let var = local_ptr(s.local);
    let cond = new_bb("for.cond".ptr);
    let body = new_bb("for.body".ptr);
    let step = new_bb("for.step".ptr);
    let end = new_bb("for.end".ptr);
    if u.notnil(s.e2) {
        let lo = gen(s.e);
        let hi = gen(s.e2);
        let iv = alloca_in_entry(lty(vt), "for.i".ptr);
        _ = st(lo, iv);
        br(cond);
        at_end(cond);
        let i = ld(lty(vt), iv);
        let mut p = L.IntULT;
        if vt.is_signed {
            p = L.IntSLT;
        }
        condbr(icmp(p, i, hi), body, end);
        at_end(body);
        _ = st(i, var);
        push_loop(end, step);
        gen_block(s.body);
        F.loops.len -= 1;
        if not terminated() {
            br(step);
        }
        at_end(step);
        let i2 = ld(lty(vt), iv);
        _ = st(b_add(i2, ci(lty(vt), 1)), iv);
        br(cond);
        at_end(end);
        return;
    }
    let ctt = ct(s.e.ty);
    let mut ptr = vnil();
    let mut len = vnil();
    if ctt.kind == ast.TY_SLICE {
        let sv = gen(s.e);
        ptr = extract(sv, 0);
        len = extract(sv, 1);
    } else {
        ptr = addr_or_spill(s.e);
        len = usz(ctt.len);
    }
    let iv = alloca_in_entry(T_isz, "for.idx".ptr);
    _ = st(usz(0), iv);
    br(cond);
    at_end(cond);
    let i = ld(T_isz, iv);
    condbr(icmp(L.IntULT, i, len), body, end);
    at_end(body);
    let ep = gep1(lty(vt), ptr, i);
    store(vt, load(vt, ep, ctt.kind == ast.TY_SLICE), var, false);
    push_loop(end, step);
    gen_block(s.body);
    F.loops.len -= 1;
    if not terminated() {
        br(step);
    }
    at_end(step);
    let i2 = ld(T_isz, iv);
    _ = st(b_add(i2, usz(1)), iv);
    br(cond);
    at_end(end);
}

fn gen_match(s: *mut ast.Stmt) void {
    let t = ct(s.e.ty);
    let end = new_bb("match.end".ptr);
    let n = s.arms.len;
    let bbs = u.newn.[*mut L.BB](n + 1);
    let mut else_idx: i64 = -1;
    for i in 0..n {
        bbs[i] = new_bb("match.arm".ptr);
        if (s.arms.at(i).kind == ast.P_ELSE) and (else_idx < 0) {
            else_idx = @int_cast(i64, i);
        }
    }
    let mut payload_ptr = vnil();
    if t.kind == ast.TY_UNION {
        let up = addr_or_spill(s.e);
        let tag = ld(lty(ty.union_tag_type(t)), sgep(lty(t), up, 0));
        if L.LLVMCountStructElementTypes(lty(t)) > 1 {
            payload_ptr = sgep(lty(t), up, 1);
        }
        let nv = t.decl.variants.len;
        let sw = L.LLVMBuildSwitch(B, tag, trap_block(T_UNIONTAG), @int_cast(u32, nv));
        let covered = u.newn.[bool](nv + 1);
        for i in 0..n {
            let a = s.arms.at(i);
            if (a.kind == ast.P_VARIANT) and not covered[@int_cast(usize, a.variant)] {
                covered[@int_cast(usize, a.variant)] = true;
                L.LLVMAddCase(sw, ci(lty(ty.union_tag_type(t)), @int_cast(u64, a.variant)), bbs[i]);
            }
        }
        if else_idx >= 0 {
            for v in 0..nv {
                if not covered[v] {
                    L.LLVMAddCase(sw, ci(lty(ty.union_tag_type(t)), @as(u64, v)), bbs[@int_cast(usize, else_idx)]);
                }
            }
        }
    } else if (O.unit == CG_MODULE) and (n > 0) and (ct(s.e.ty).kind == ast.TY_ERROR) {
        // Error codes are link-time values here: compare in arm order.
        let v = gen(s.e);
        for i in 0..n {
            let a = s.arms.at(i);
            if a.kind != ast.P_ERROR {
                continue;
            }
            let next = new_bb("match.next".ptr);
            condbr(icmp(L.IntEQ, v, errcode_value(a.error_code)), bbs[i], next);
            at_end(next);
        }
        if else_idx >= 0 {
            br(bbs[@int_cast(usize, else_idx)]);
        } else {
            br(trap_block(T_TRAP));
        }
    } else {
        let v = gen(s.e);
        let mut dflt = bbnil();
        if else_idx >= 0 {
            dflt = bbs[@int_cast(usize, else_idx)];
        } else {
            dflt = trap_block(T_TRAP);
        }
        let sw = L.LLVMBuildSwitch(B, v, dflt, @int_cast(u32, n));
        let vt_ = L.LLVMTypeOf(v);
        let mut seen = u.List.[u.I128].{ data: u.nil.[u.I128](), len: 0, cap: 0 };
        for i in 0..n {
            let a = s.arms.at(i);
            let mut key = u.I128.zero();
            if a.kind == ast.P_INT {
                key = a.ival;
            } else if a.kind == ast.P_BOOL {
                if a.bval {
                    key = u.I128.of(1);
                }
            } else if a.kind == ast.P_ERROR {
                key = u.I128.ofu(@as(u64, a.error_code));
            } else {
                continue;
            }
            let mut dup = false;
            for kk in 0..seen.len {
                if seen.get(kk).eq(key) {
                    dup = true;
                }
            }
            if dup {
                continue;
            }
            seen.push(key);
            L.LLVMAddCase(sw, cint(vt_, key), bbs[i]);
        }
        u.free(@ptr_cast(*mut void, seen.data));
    }
    for i in 0..n {
        let a = s.arms.at(i);
        at_end(bbs[i]);
        if u.notnil(a.bind_local) and u.notnil(payload_ptr) {
            let pt = ty.type_variant(t, @int_cast(usize, a.variant));
            store(pt, load(pt, payload_ptr, true), local_ptr(a.bind_local), false);
        }
        gen_block(a.body);
        if not terminated() {
            br(end);
        }
    }
    at_end(end);
}

fn gen_stmt_inner(s: *mut ast.Stmt) void {
    let k = s.kind;
    if k == ast.S_LET {
        let v = gen(s.e);
        if terminated() or u.isnil(v) {
            return;
        }
        store(ct(s.local.ty), v, local_ptr(s.local), false);
    } else if (k == ast.S_EXPR) or (k == ast.S_DISCARD) {
        _ = gen(s.e);
    } else if k == ast.S_ASSIGN {
        let t = ct(s.e.ty);
        let addr = gen_addr(s.e);
        let un = via_pointer(s.e);
        if s.op == lx.TK_EQ {
            let v = gen(s.e2);
            if u.isnil(v) {
                return;
            }
            store(t, v, addr, un);
            return;
        }
        let old = load(t, addr, un);
        let r = gen(s.e2);
        let nv = gen_binop(s.e3.op, t, old, r);
        store(t, nv, addr, un);
    } else if k == ast.S_IF {
        gen_if(s);
    } else if k == ast.S_WHILE {
        let cond = new_bb("while.cond".ptr);
        let body = new_bb("while.body".ptr);
        let end = new_bb("while.end".ptr);
        br(cond);
        at_end(cond);
        let c = gen(s.e);
        condbr(c, body, end);
        at_end(body);
        push_loop(end, cond);
        gen_block(s.body);
        F.loops.len -= 1;
        if not terminated() {
            br(cond);
        }
        at_end(end);
    } else if k == ast.S_LOOP {
        let body = new_bb("loop.body".ptr);
        let end = new_bb("loop.end".ptr);
        br(body);
        at_end(body);
        push_loop(end, body);
        gen_block(s.body);
        F.loops.len -= 1;
        if not terminated() {
            br(body);
        }
        at_end(end);
    } else if k == ast.S_FOR {
        gen_for(s);
    } else if k == ast.S_MATCH {
        gen_match(s);
    } else if k == ast.S_RETURN {
        gen_return(s.e);
    } else if k == ast.S_BREAK {
        gen_break(true);
    } else if k == ast.S_CONTINUE {
        gen_break(false);
    } else if k == ast.S_DEFER {
        F.defers.push(s.body);
    } else if k == ast.S_WASM {
        _ = gen_wasm(&mut s.wins, tnil());
    } else if k == ast.S_BLOCK {
        gen_block(s);
    }
}

// Wrappers that track the source span of the code being generated, for
// trap sites.
fn gen(e: *mut ast.Expr) *mut L.Value {
    let saved = g_cur_span;
    g_cur_span = e.span;
    let v = gen_inner(e);
    g_cur_span = saved;
    return v;
}

fn gen_addr(e: *mut ast.Expr) *mut L.Value {
    let saved = g_cur_span;
    g_cur_span = e.span;
    let v = gen_addr_inner(e);
    g_cur_span = saved;
    return v;
}

fn gen_stmt(s: *mut ast.Stmt) void {
    let saved = g_cur_span;
    g_cur_span = s.span;
    gen_stmt_inner(s);
    g_cur_span = saved;
}

// ---------------------------------------------------------------------------
// Function bodies
// ---------------------------------------------------------------------------
fn gen_fn_body(fi: *mut FnInst) void {
    let d = fi.decl;
    let stt = u.new.[FnState]();
    stt.inst = fi;
    stt.decl = d;
    stt.from = d.all_gparams;
    stt.to = fi.gargs;
    stt.fnv = fi.fnv;
    stt.locals = u.newn.[*mut L.Value](d.locals.len + 1);
    let saved = F;
    F = stt;
    stt.ret = fi.fnty.ret;
    stt.entry = L.LLVMAppendBasicBlockInContext(ctx, fi.fnv, "entry".ptr);
    let start = L.LLVMAppendBasicBlockInContext(ctx, fi.fnv, "start".ptr);
    at_end(start);
    let mut k: u32 = 0;
    if fi.sret {
        stt.sret_ptr = L.LLVMGetParam(fi.fnv, k);
        k += 1;
    }
    for i in 0..d.params.len {
        let l = d.params.at(i).local;
        let pt = fi.fnty.params[i];
        let p = L.LLVMGetParam(fi.fnv, k);
        k += 1;
        if by_ptr(pt) {
            stt.locals[@int_cast(usize, l.index)] = p;  // params are immutable: use the caller's copy directly
        } else {
            store(pt, p, local_ptr(l), false);
        }
    }
    gen_block(d.body);
    if not terminated() {
        if stt.ret.kind == ast.TY_VOID {
            _ = L.LLVMBuildRetVoid(B);
        } else if (stt.ret.kind == ast.TY_ERRUNION) and (stt.ret.elem.kind == ast.TY_VOID) {
            emit_ret(errunion_value(stt.ret, ci(T_i16, 0), vnil()));
        } else {
            _ = L.LLVMBuildUnreachable(B);
        }
    }
    at_end(stt.entry);
    br(start);
    // Delete blocks with no predecessors (continuations after return/break/trap).
    // Leaving them for the backend makes -O0 code generation several times slower.
    let mut changed = true;
    while changed {
        changed = false;
        let mut bb = L.LLVMGetNextBasicBlock(stt.entry);
        while u.notnil(bb) {
            let next = L.LLVMGetNextBasicBlock(bb);
            if u.isnil(L.LLVMGetFirstUse(L.LLVMBasicBlockAsValue(bb))) {
                L.LLVMDeleteBasicBlock(bb);
                changed = true;
            }
            bb = next;
        }
    }
    u.free(@ptr_cast(*mut void, stt.defers.data));
    u.free(@ptr_cast(*mut void, stt.loops.data));
    F = saved;
}

// ---------------------------------------------------------------------------
// Inline WebAssembly -> LLVM IR
// ---------------------------------------------------------------------------
fn wasm_llty(c: u8) *mut L.LType {
    if c == 'i' {
        return T_i32;
    }
    if c == 'l' {
        return T_i64;
    }
    if c == 'f' {
        return T_f32;
    }
    if c == 'd' {
        return T_f64;
    }
    return T_v128;
}

fn to_wasm_val(t: *mut ast.Type, v: *mut L.Value) *mut L.Value {
    let k = t.kind;
    if k == ast.TY_BOOL {
        return zext(v, T_i32);
    }
    if k == ast.TY_INT {
        if t.bits < 32 {
            if t.is_signed {
                return sext(v, T_i32);
            }
            return zext(v, T_i32);
        }
        return v;
    }
    if k == ast.TY_PTR {
        return ptrtoint(v, T_isz);
    }
    if k == ast.TY_VEC {
        return bitcast(v, T_v128);
    }
    return v;
}

fn from_wasm_val(t: *mut ast.Type, v: *mut L.Value) *mut L.Value {
    let k = t.kind;
    if k == ast.TY_BOOL {
        return icmp(L.IntNE, v, ci32(0));
    }
    if k == ast.TY_INT {
        if t.bits < 32 {
            return trunc(v, lty(t));
        }
        return v;
    }
    if k == ast.TY_PTR {
        return inttoptr(v, T_ptr);
    }
    if k == ast.TY_VEC {
        return bitcast(v, lty(t));
    }
    return v;
}

fn shape_ty(prefix: []u8) *mut L.LType {
    if u.str_eq(prefix, "i8x16") {
        return L.LLVMVectorType(T_i8, 16);
    }
    if u.str_eq(prefix, "i16x8") {
        return L.LLVMVectorType(T_i16, 8);
    }
    if u.str_eq(prefix, "i32x4") {
        return L.LLVMVectorType(T_i32, 4);
    }
    if u.str_eq(prefix, "i64x2") {
        return L.LLVMVectorType(T_i64, 2);
    }
    if u.str_eq(prefix, "f32x4") {
        return L.LLVMVectorType(T_f32, 4);
    }
    if u.str_eq(prefix, "f64x2") {
        return L.LLVMVectorType(T_f64, 2);
    }
    return T_v128;
}

fn imm_int(w: *mut ast.WasmInstr, i: usize) i64 {
    let s = w.imms.at(i).name;
    if s[0] == '-' {
        return u.c_strtoll(s).v;
    }
    return u.c_strtoull(s).v;
}

fn imm_float(w: *mut ast.WasmInstr) f64 {
    let s = w.imms.at(0).name;
    if u.str_eq(s, "nan") {
        return u.c_strtod("nan").v;
    }
    if u.str_eq(s, "inf") or u.str_eq(s, "+inf") {
        return u.c_strtod("inf").v;
    }
    if u.str_eq(s, "-inf") {
        return u.c_strtod("-inf").v;
    }
    return u.c_strtod(s).v;
}

fn mem_addr(w: *mut ast.WasmInstr, base: *mut L.Value) *mut L.Value {
    let mut off: i64 = 0;
    for i in 0..w.imms.len {
        let s = w.imms.at(i).name;
        if u.starts(s, "offset=") {
            off = u.c_strtoll(s[7..]).v;
        }
    }
    let mut a = base;
    if off != 0 {
        a = b_add(a, ci(L.LLVMTypeOf(a), @bit_cast(u64, off)));
    }
    return inttoptr(a, T_ptr);
}

fn bool_i32(c: *mut L.Value) *mut L.Value {
    return zext(c, T_i32);
}

struct WStack {
    v: [256]*mut L.Value,
    n: usize,
}

fn wpop(s: *mut WStack) *mut L.Value {
    s.n -= 1;
    return s.v[s.n];
}

fn wpush(s: *mut WStack, x: *mut L.Value) void {
    s.v[s.n] = x;
    s.n += 1;
}

fn gen_wasm(ins: *mut u.List.[ast.WasmInstr], result: *mut ast.Type) *mut L.Value {
    let S = u.new.[WStack]();
    for ii in 0..ins.len {
        let w = ins.at(ii);
        let op = wops.lookup(w.op);
        let pre = op.prefix;
        let o = op.op;
        if u.str_eq(w.op, "nop") {
            continue;
        }
        if u.str_eq(w.op, "unreachable") {
            emit_trap(T_TRAP);
            if u.notnil(result) and (result.kind != ast.TY_VOID) {
                return L.LLVMGetPoison(vty(result));
            }
            return vnil();
        }
        if u.str_eq(w.op, "drop") {
            S.n -= 1;
            continue;
        }
        if u.str_eq(w.op, "select") {
            let c = wpop(S);
            let b = wpop(S);
            let a = wpop(S);
            wpush(S, sel(icmp(L.IntNE, c, ci32(0)), a, b));
            continue;
        }
        if u.str_eq(pre, "local") {
            let l = w.local;
            let lt = ct(l.ty);
            if u.str_eq(o, "get") {
                wpush(S, to_wasm_val(lt, load(lt, local_ptr(l), false)));
            } else {
                let v = wpop(S);
                store(lt, from_wasm_val(lt, v), local_ptr(l), false);
                if u.str_eq(o, "tee") {
                    wpush(S, v);
                }
            }
            continue;
        }
        let is_int = u.str_eq(pre, "i32") or u.str_eq(pre, "i64");
        let is_flt = u.str_eq(pre, "f32") or u.str_eq(pre, "f64");
        let mut T = u.nil.[L.LType]();
        if is_int or is_flt {
            let mut own: u8 = 'd';
            if u.str_eq(pre, "i32") {
                own = 'i';
            } else if u.str_eq(pre, "i64") {
                own = 'l';
            } else if u.str_eq(pre, "f32") {
                own = 'f';
            }
            T = wasm_llty(own);
        }
        let is_cmp = u.str_eq(o, "eq") or u.str_eq(o, "ne") or u.starts(o, "lt") or u.starts(o, "gt") or
                     u.starts(o, "le") or u.starts(o, "ge");
        if u.str_eq(o, "const") {
            if is_int {
                wpush(S, L.LLVMConstInt(T, @bit_cast(u64, imm_int(w, 0)), 1));
            } else {
                wpush(S, L.LLVMConstReal(T, imm_float(w)));
            }
            continue;
        }
        if op.imm == wops.WIMM_MEM {
            let is_store = u.starts(o, "store");
            let mut memt = u.nil.[L.LType]();
            if u.str_eq(pre, "v128") {
                memt = T_v128;
            } else if op.mem_bytes == 1 {
                memt = T_i8;
            } else if op.mem_bytes == 2 {
                memt = T_i16;
            } else if op.mem_bytes == 4 {
                memt = T_i32;
                if is_flt {
                    memt = T_f32;
                }
            } else {
                memt = T_i64;
                if is_flt {
                    memt = T_f64;
                }
            }
            let mut valt = T;
            if u.str_eq(pre, "v128") {
                valt = T_v128;
            }
            if is_store {
                let mut v = wpop(S);
                let base = wpop(S);
                if memt != valt {
                    v = trunc(v, memt);
                }
                let s = st(v, mem_addr(w, base));
                L.LLVMSetAlignment(s, 1);
            } else {
                let base = wpop(S);
                let mut v = ld(memt, mem_addr(w, base));
                L.LLVMSetAlignment(v, 1);
                if memt != valt {
                    if op.mem_signed {
                        v = sext(v, valt);
                    } else {
                        v = zext(v, valt);
                    }
                }
                wpush(S, v);
            }
            continue;
        }
        if is_int {
            let mut bits: u64 = 64;
            if u.str_eq(pre, "i32") {
                bits = 32;
            }
            if (op.ins.len == 2) and not is_cmp {
                let b = wpop(S);
                let a = wpop(S);
                let mut r = vnil();
                if u.str_eq(o, "add") {
                    r = b_add(a, b);
                } else if u.str_eq(o, "sub") {
                    r = b_sub(a, b);
                } else if u.str_eq(o, "mul") {
                    r = b_mul(a, b);
                } else if u.str_eq(o, "and") {
                    r = b_and(a, b);
                } else if u.str_eq(o, "or") {
                    r = b_or(a, b);
                } else if u.str_eq(o, "xor") {
                    r = b_xor(a, b);
                } else if u.str_eq(o, "div_u") or u.str_eq(o, "rem_u") or u.str_eq(o, "div_s") or u.str_eq(o, "rem_s") {
                    check_or_trap(icmp(L.IntNE, b, L.LLVMConstNull(T)), T_DIVZERO);
                    if u.str_eq(o, "div_u") {
                        r = L.LLVMBuildUDiv(B, a, b, NN());
                    } else if u.str_eq(o, "rem_u") {
                        r = L.LLVMBuildURem(B, a, b, NN());
                    } else {
                        let m1 = icmp(L.IntEQ, b, L.LLVMConstAllOnes(T));
                        if u.str_eq(o, "div_s") {
                            check_or_trap(b_not(b_and(m1, icmp(L.IntEQ, a, int_min(T)))), T_OVERFLOW);
                            r = L.LLVMBuildSDiv(B, a, b, NN());
                        } else {
                            r = L.LLVMBuildSRem(B, a, sel(m1, ci(T, 1), b), NN());
                        }
                    }
                } else if u.str_eq(o, "shl") or u.str_eq(o, "shr_s") or u.str_eq(o, "shr_u") {
                    let k = b_and(b, ci(T, bits - 1));
                    if u.str_eq(o, "shl") {
                        r = L.LLVMBuildShl(B, a, k, NN());
                    } else if u.str_eq(o, "shr_s") {
                        r = L.LLVMBuildAShr(B, a, k, NN());
                    } else {
                        r = L.LLVMBuildLShr(B, a, k, NN());
                    }
                } else if u.str_eq(o, "rotl") or u.str_eq(o, "rotr") {
                    let mut tys: [1]*mut L.LType = [T];
                    let mut args: [3]*mut L.Value = [a, a, b];
                    r = intrinsic(u.pick(u.str_eq(o, "rotl"), "llvm.fshl", "llvm.fshr"), tys[..], args[..]);
                }
                if u.isnil(r) {
                    internal_error(u.f1("wasm op %s", w.op));
                }
                wpush(S, r);
                continue;
            }
            if is_cmp {
                let b = wpop(S);
                let a = wpop(S);
                let mut p = L.IntEQ;
                if u.str_eq(o, "eq") {
                    p = L.IntEQ;
                } else if u.str_eq(o, "ne") {
                    p = L.IntNE;
                } else if u.str_eq(o, "lt_s") {
                    p = L.IntSLT;
                } else if u.str_eq(o, "lt_u") {
                    p = L.IntULT;
                } else if u.str_eq(o, "gt_s") {
                    p = L.IntSGT;
                } else if u.str_eq(o, "gt_u") {
                    p = L.IntUGT;
                } else if u.str_eq(o, "le_s") {
                    p = L.IntSLE;
                } else if u.str_eq(o, "le_u") {
                    p = L.IntULE;
                } else if u.str_eq(o, "ge_s") {
                    p = L.IntSGE;
                } else if u.str_eq(o, "ge_u") {
                    p = L.IntUGE;
                }
                wpush(S, bool_i32(icmp(p, a, b)));
                continue;
            }
            let a = wpop(S);
            let mut r = vnil();
            if u.str_eq(o, "clz") or u.str_eq(o, "ctz") {
                let mut tys: [1]*mut L.LType = [T];
                let mut args: [2]*mut L.Value = [a, ci(T_i1, 0)];
                r = intrinsic(u.pick(u.str_eq(o, "clz"), "llvm.ctlz", "llvm.cttz"), tys[..], args[..]);
            } else if u.str_eq(o, "popcnt") {
                r = intr1("llvm.ctpop", a);
            } else if u.str_eq(o, "eqz") {
                r = bool_i32(icmp(L.IntEQ, a, L.LLVMConstNull(L.LLVMTypeOf(a))));
            } else if u.str_eq(o, "extend8_s") {
                r = sext(trunc(a, T_i8), T);
            } else if u.str_eq(o, "extend16_s") {
                r = sext(trunc(a, T_i16), T);
            } else if u.str_eq(o, "extend32_s") {
                r = sext(trunc(a, T_i32), T);
            } else if u.str_eq(o, "wrap_i64") {
                r = trunc(a, T_i32);
            } else if u.str_eq(o, "extend_i32_s") {
                r = sext(a, T_i64);
            } else if u.str_eq(o, "extend_i32_u") {
                r = zext(a, T_i64);
            } else if u.starts(o, "reinterpret") {
                r = bitcast(a, T);
            } else if u.starts(o, "trunc_sat") {
                let mut tys: [2]*mut L.LType = [T, L.LLVMTypeOf(a)];
                let mut args: [1]*mut L.Value = [a];
                r = intrinsic(u.pick(o[o.len - 1] == 's', "llvm.fptosi.sat", "llvm.fptoui.sat"), tys[..], args[..]);
            }
            if u.isnil(r) {
                internal_error(u.f1("wasm op %s", w.op));
            }
            wpush(S, r);
            continue;
        }
        if is_flt {
            if op.ins.len == 2 {
                let b = wpop(S);
                let a = wpop(S);
                let mut r = vnil();
                if op.out == 'i' {
                    let mut p = L.RealOEQ;
                    if u.str_eq(o, "ne") {
                        p = L.RealUNE;
                    } else if u.str_eq(o, "lt") {
                        p = L.RealOLT;
                    } else if u.str_eq(o, "gt") {
                        p = L.RealOGT;
                    } else if u.str_eq(o, "le") {
                        p = L.RealOLE;
                    } else if u.str_eq(o, "ge") {
                        p = L.RealOGE;
                    }
                    r = bool_i32(fcmp(p, a, b));
                } else if u.str_eq(o, "add") {
                    r = L.LLVMBuildFAdd(B, a, b, NN());
                } else if u.str_eq(o, "sub") {
                    r = L.LLVMBuildFSub(B, a, b, NN());
                } else if u.str_eq(o, "mul") {
                    r = L.LLVMBuildFMul(B, a, b, NN());
                } else if u.str_eq(o, "div") {
                    r = L.LLVMBuildFDiv(B, a, b, NN());
                } else if u.str_eq(o, "min") {
                    r = intr2("llvm.minimum", a, b);
                } else if u.str_eq(o, "max") {
                    r = intr2("llvm.maximum", a, b);
                } else if u.str_eq(o, "copysign") {
                    r = intr2("llvm.copysign", a, b);
                }
                if u.isnil(r) {
                    internal_error(u.f1("wasm op %s", w.op));
                }
                wpush(S, r);
                continue;
            }
            let a = wpop(S);
            let mut r = vnil();
            if u.str_eq(o, "abs") {
                r = intr1("llvm.fabs", a);
            } else if u.str_eq(o, "neg") {
                r = L.LLVMBuildFNeg(B, a, NN());
            } else if u.str_eq(o, "sqrt") {
                r = intr1("llvm.sqrt", a);
            } else if u.str_eq(o, "ceil") {
                r = intr1("llvm.ceil", a);
            } else if u.str_eq(o, "floor") {
                r = intr1("llvm.floor", a);
            } else if u.str_eq(o, "trunc") {
                r = intr1("llvm.trunc", a);
            } else if u.str_eq(o, "nearest") {
                r = intr1("llvm.roundeven", a);
            } else if u.str_eq(o, "demote_f64") {
                r = L.LLVMBuildFPTrunc(B, a, T_f32, NN());
            } else if u.str_eq(o, "promote_f32") {
                r = L.LLVMBuildFPExt(B, a, T_f64, NN());
            } else if u.starts(o, "reinterpret") {
                r = bitcast(a, T);
            } else if u.starts(o, "convert_") {
                if o[o.len - 1] == 's' {
                    r = L.LLVMBuildSIToFP(B, a, T, NN());
                } else {
                    r = L.LLVMBuildUIToFP(B, a, T, NN());
                }
            }
            if u.isnil(r) {
                internal_error(u.f1("wasm op %s", w.op));
            }
            wpush(S, r);
            continue;
        }
        // SIMD
        let vt = shape_ty(pre);
        if u.str_eq(pre, "v128") {
            if u.str_eq(o, "not") {
                wpush(S, b_not(wpop(S)));
                continue;
            }
            let b = wpop(S);
            let a = wpop(S);
            let mut r = vnil();
            if u.str_eq(o, "and") {
                r = b_and(a, b);
            } else if u.str_eq(o, "or") {
                r = b_or(a, b);
            } else if u.str_eq(o, "xor") {
                r = b_xor(a, b);
            } else {
                r = b_and(a, b_not(b));
            }
            wpush(S, r);
            continue;
        }
        let vf = pre[0] == 'f';
        let lane = L.LLVMGetElementType(vt);
        if u.str_eq(o, "splat") {
            let mut x = wpop(S);
            if not vf and (L.LLVMGetIntTypeWidth(lane) < 32) {
                x = trunc(x, lane);
            }
            wpush(S, bitcast(splat(vt, x, L.LLVMGetVectorSize(vt)), T_v128));
            continue;
        }
        if u.starts(o, "extract_lane") {
            let v = bitcast(wpop(S), vt);
            let mut x = L.LLVMBuildExtractElement(B, v, ci32(@bit_cast(u64, imm_int(w, 0))), NN());
            if not vf and (L.LLVMGetIntTypeWidth(lane) < 32) {
                if o[o.len - 1] == 's' {
                    x = sext(x, T_i32);
                } else {
                    x = zext(x, T_i32);
                }
            }
            wpush(S, x);
            continue;
        }
        if u.str_eq(o, "replace_lane") {
            let mut x = wpop(S);
            let mut v = bitcast(wpop(S), vt);
            if not vf and (L.LLVMGetIntTypeWidth(lane) < 32) {
                x = trunc(x, lane);
            }
            v = L.LLVMBuildInsertElement(B, v, x, ci32(@bit_cast(u64, imm_int(w, 0))), NN());
            wpush(S, bitcast(v, T_v128));
            continue;
        }
        if op.ins.len == 1 {
            let a = bitcast(wpop(S), vt);
            let mut r = vnil();
            if u.str_eq(o, "neg") {
                if vf {
                    r = L.LLVMBuildFNeg(B, a, NN());
                } else {
                    r = L.LLVMBuildNeg(B, a, NN());
                }
            } else if u.str_eq(o, "sqrt") {
                r = intr1("llvm.sqrt", a);
            } else {
                r = intr1("llvm.fabs", a);
            }
            wpush(S, bitcast(r, T_v128));
            continue;
        }
        let b = bitcast(wpop(S), vt);
        let a = bitcast(wpop(S), vt);
        let mut r = vnil();
        if u.str_eq(o, "add") {
            if vf {
                r = L.LLVMBuildFAdd(B, a, b, NN());
            } else {
                r = b_add(a, b);
            }
        } else if u.str_eq(o, "sub") {
            if vf {
                r = L.LLVMBuildFSub(B, a, b, NN());
            } else {
                r = b_sub(a, b);
            }
        } else if u.str_eq(o, "mul") {
            if vf {
                r = L.LLVMBuildFMul(B, a, b, NN());
            } else {
                r = b_mul(a, b);
            }
        } else if u.str_eq(o, "div") {
            r = L.LLVMBuildFDiv(B, a, b, NN());
        } else if u.str_eq(o, "min") {
            r = intr2("llvm.minimum", a, b);
        } else {
            r = intr2("llvm.maximum", a, b);
        }
        wpush(S, bitcast(r, T_v128));
    }
    if u.notnil(result) and (result.kind != ast.TY_VOID) and (S.n > 0) {
        return from_wasm_val(result, S.v[S.n - 1]);
    }
    return vnil();
}

// ---------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------
fn host_is_aarch64() bool {
    let t = u.from_c(L.LLVMGetDefaultTargetTriple());
    return u.starts(t, "aarch64") or u.starts(t, "arm64");
}

fn setup_target() bool {
    let triple = ty.g_target.triple;
    if ty.g_target.is_wasm {
        L.LLVMInitializeWebAssemblyTargetInfo();
        L.LLVMInitializeWebAssemblyTarget();
        L.LLVMInitializeWebAssemblyTargetMC();
        L.LLVMInitializeWebAssemblyAsmPrinter();
    } else if host_is_aarch64() {
        L.LLVMInitializeAArch64TargetInfo();
        L.LLVMInitializeAArch64Target();
        L.LLVMInitializeAArch64TargetMC();
        L.LLVMInitializeAArch64AsmPrinter();
    } else {
        L.LLVMInitializeX86TargetInfo();
        L.LLVMInitializeX86Target();
        L.LLVMInitializeX86TargetMC();
        L.LLVMInitializeX86AsmPrinter();
    }
    let mut target = u.nil.[L.Target]();
    let mut err = u.nil.[u8]();
    if L.LLVMGetTargetFromTriple(u.cz(triple), &mut target, &mut err) != 0 {
        u.eprint(u.f2("phalange: cannot find LLVM target '%s': %s\n", triple, u.from_c(err)));
        L.LLVMDisposeMessage(err);
        return false;
    }
    let mut features: []u8 = "";
    if ty.g_target.is_wasm {
        features = "+simd128,+bulk-memory,+nontrapping-fptoint,+sign-ext,+mutable-globals,+atomics";
    }
    let ol = O.opt_level;
    let mut cpu = O.cpu;
    if u.isnos(cpu) {
        if ty.g_target.is_wasm {
            cpu = "generic";
        } else {
            cpu = u.from_c(L.LLVMGetHostCPUName());
        }
    }
    let mut reloc = L.RelocPIC;
    if ty.g_target.is_wasm {
        reloc = L.RelocStatic;
    }
    TM = L.LLVMCreateTargetMachine(target, u.cz(triple), u.cz(cpu), u.cz(features), ol, reloc, L.CodeModelDefault);
    TD = L.LLVMCreateTargetDataLayout(TM);
    L.LLVMSetTarget(M, u.cz(triple));
    L.LLVMSetModuleDataLayout(M, TD);
    return true;
}

// Freestanding wasm has no compiler-rt, but LLVM lowers 64-bit multiply
// overflow checks (and other 128-bit multiplies) to the __multi3 libcall.
// Provide it, built from 64-bit operations only and kept out of the
// optimizer so it cannot be turned back into a 128-bit multiply.
fn emit_multi3() void {
    let mut ps: [5]*mut L.LType = [T_ptr, T_i64, T_i64, T_i64, T_i64];
    let fty = fn_type(T_void, ps[..], false);
    let f = L.LLVMAddFunction(M, "__multi3".ptr, fty);
    L.LLVMSetLinkage(f, L.WeakAnyLinkage);
    L.LLVMSetVisibility(f, L.HiddenVisibility);
    add_fn_attr(f, "optnone");
    add_fn_attr(f, "noinline");
    let bb = L.LLVMAppendBasicBlockInContext(ctx, f, "entry".ptr);
    at_end(bb);
    let ret = L.LLVMGetParam(f, 0);
    let al = L.LLVMGetParam(f, 1);
    let ah = L.LLVMGetParam(f, 2);
    let bl = L.LLVMGetParam(f, 3);
    let bh = L.LLVMGetParam(f, 4);
    let m32 = ci(T_i64, 0xffffffff);
    let s32 = ci(T_i64, 32);
    let a0 = b_and(al, m32);
    let a1 = L.LLVMBuildLShr(B, al, s32, NN());
    let b0 = b_and(bl, m32);
    let b1 = L.LLVMBuildLShr(B, bl, s32, NN());
    let p00 = b_mul(a0, b0);
    let p01 = b_mul(a0, b1);
    let p10 = b_mul(a1, b0);
    let p11 = b_mul(a1, b1);
    // (in the order the C implementation's compiler evaluates the arguments: right to left)
    let q10 = b_and(p10, m32);
    let q01 = b_and(p01, m32);
    let mid = b_add(b_add(L.LLVMBuildLShr(B, p00, s32, NN()), q01), q10);
    let smid = L.LLVMBuildLShr(B, mid, s32, NN());
    let h2 = b_add(L.LLVMBuildLShr(B, p10, s32, NN()), smid);
    let h1 = b_add(p11, L.LLVMBuildLShr(B, p01, s32, NN()));
    let mut hi = b_add(h1, h2);
    let shm = L.LLVMBuildShl(B, mid, s32, NN());
    let lo = b_or(b_and(p00, m32), shm);
    let m2 = b_mul(ah, bl);
    let m1 = b_mul(al, bh);
    hi = b_add(hi, b_add(m1, m2));
    _ = st(lo, ret);
    _ = st(hi, gep1(T_i8, ret, ci32(8)));
    _ = L.LLVMBuildRetVoid(B);
}

// A small exported function whose body the caller builds.
fn add_export(name: []u8, ret: *mut L.LType, ps: []mut *mut L.LType) *mut L.Value {
    let fty = fn_type(ret, ps, false);
    let f = L.LLVMAddFunction(M, u.cz(name), fty);
    if ty.g_target.is_wasm {
        add_str_attr(f, "wasm-export-name", name);
    }
    g_export_names.push(u.intern(name));
    at_end(L.LLVMAppendBasicBlockInContext(ctx, f, "entry".ptr));
    return f;
}

// phalange_shadow_stack(): the calling thread's shadow stack record (see
// ss_ptr), or null when built with --stack-traces=off.
fn emit_shadow_stack_export() void {
    _ = add_export("phalange_shadow_stack", T_ptr, no_tys());
    if stack_traces_on() {
        _ = L.LLVMBuildRet(B, ss_ptr());
    } else {
        _ = L.LLVMBuildRet(B, L.LLVMConstNull(T_ptr));
    }
}

// phalange_trap_info(): address of {site, kind, a, b, alen, blen} (64-bit
// fields); `site` points at {code, file_id, start, end, line, col, file, text}.
fn emit_trap_info_export() void {
    _ = add_export("phalange_trap_info", T_ptr, no_tys());
    _ = L.LLVMBuildRet(B, trap_info_global());
}

// Test binaries (spec §18): tests are listed and run through exports.
//   phalange_test_count() u32
//   phalange_test_name(i) *u8        NUL-terminated
//   phalange_test_expect(i) i32      -1 must not trap, -2 any trap, n: trap Tn
//   phalange_test_run(i) u32         0, or the error code the test returned
//   phalange_error_name(code) *u8    NUL-terminated
fn emit_test_exports() void {
    let tests = sema.g_prog.tests;
    let n = tests.len;
    _ = add_export("phalange_test_count", T_i32, no_tys());
    _ = L.LLVMBuildRet(B, ci32(@as(u64, n)));

    let names = u.newn.[*mut L.Value](n + 1);
    let expects = u.newn.[*mut L.Value](n + 1);
    for i in 0..n {
        let d = tests.get(i);
        names[i] = string_ptr(cstr_prefix(d.test_name));
        expects[i] = L.LLVMConstInt(T_i32, @bit_cast(u64, @as(i64, d.test_trap)), 1);
    }
    let ntab = L.LLVMAddGlobal(M, L.LLVMArrayType2(T_ptr, @as(u64, n) + 1), "phalange.test_names".ptr);
    names[n] = L.LLVMConstNull(T_ptr);
    L.LLVMSetInitializer(ntab, L.LLVMConstArray2(T_ptr, &mut names[0], @as(u64, n) + 1));
    L.LLVMSetLinkage(ntab, L.PrivateLinkage);
    let etab = L.LLVMAddGlobal(M, L.LLVMArrayType2(T_i32, @as(u64, n) + 1), "phalange.test_expect".ptr);
    expects[n] = ci32(0);
    L.LLVMSetInitializer(etab, L.LLVMConstArray2(T_i32, &mut expects[0], @as(u64, n) + 1));
    L.LLVMSetLinkage(etab, L.PrivateLinkage);

    let mut pi: [1]*mut L.LType = [T_i32];
    let mut f = add_export("phalange_test_name", T_ptr, pi[..]);
    _ = L.LLVMBuildRet(B, ld(T_ptr, gep2(L.LLVMArrayType2(T_ptr, @as(u64, n) + 1), ntab, ci32(0), L.LLVMGetParam(f, 0))));
    f = add_export("phalange_test_expect", T_i32, pi[..]);
    _ = L.LLVMBuildRet(B, ld(T_i32, gep2(L.LLVMArrayType2(T_i32, @as(u64, n) + 1), etab, ci32(0), L.LLVMGetParam(f, 0))));

    // phalange_test_run: a switch over the test functions.
    let run = add_export("phalange_test_run", T_i32, pi[..]);
    let dflt = L.LLVMAppendBasicBlockInContext(ctx, run, "none".ptr);
    let sw = L.LLVMBuildSwitch(B, L.LLVMGetParam(run, 0), dflt, @int_cast(u32, n));
    for i in 0..n {
        let fi = get_inst(tests.get(i), ty.no_types());
        let bb = L.LLVMAppendBasicBlockInContext(ctx, run, "test".ptr);
        L.LLVMAddCase(sw, ci32(@as(u64, i)), bb);
        at_end(bb);
        let r = call(fi.llty, fi.fnv, no_vals());
        _ = L.LLVMBuildRet(B, zext(extract(r, 0), T_i32));
    }
    at_end(dflt);
    _ = L.LLVMBuildRet(B, ci32(0xFFFF));

    let en = add_export("phalange_error_name", T_ptr, pi[..]);
    let tab = error_name_table();
    let ne = sema.g_prog.error_names.len;
    let mut code = zext(L.LLVMGetParam(en, 0), T_isz);
    code = sel(icmp(L.IntULE, code, usz(@int_cast(i64, ne))), code, usz(0));
    let stt = lty(ty.type_slice(ty.t_u8, false));
    let sl = ld(stt, gep2(L.LLVMArrayType2(stt, @as(u64, ne) + 1), tab, usz(0), code));
    _ = L.LLVMBuildRet(B, extract(sl, 0));
}

pub fn default_triple() []u8 {
    return u.dup(u.from_c(L.LLVMGetDefaultTargetTriple()));
}

// Generates code for the checked program. False on failure (errors reported
// through diag or printed to stderr).
pub fn codegen_program(o: *mut CgOptions) bool {
    O = o;
    let P = sema.g_prog;
    ctx = L.LLVMContextCreate();
    M = L.LLVMModuleCreateWithNameInContext(u.cz(u.file_of(P.root.file.id).display), ctx);
    B = L.LLVMCreateBuilderInContext(ctx);
    if not setup_target() {
        return false;
    }
    T_i1 = L.LLVMInt1TypeInContext(ctx);
    T_i8 = L.LLVMInt8TypeInContext(ctx);
    T_i16 = L.LLVMInt16TypeInContext(ctx);
    T_i32 = L.LLVMInt32TypeInContext(ctx);
    T_i64 = L.LLVMInt64TypeInContext(ctx);
    T_i128 = L.LLVMIntTypeInContext(ctx, 128);
    T_isz = tint(ty.g_target.ptr_bits);
    T_ptr = L.LLVMPointerTypeInContext(ctx, 0);
    T_void = L.LLVMVoidTypeInContext(ctx);
    T_f32 = L.LLVMFloatTypeInContext(ctx);
    T_f64 = L.LLVMDoubleTypeInContext(ctx);
    T_v128 = L.LLVMVectorType(T_i32, 4);

    if ty.g_target.is_wasm {
        emit_multi3();
    }
    let program_parts = O.unit != CG_MODULE;
    if program_parts and ty.g_target.is_wasm and g_threads {
        emit_thread_start();
    }

    if O.unit == CG_WHOLE {
        // Roots: exported functions. Everything else (functions, generic
        // instances, globals) is generated on first reference, so
        // unreachable code costs nothing in the backend.
        for i in 0..P.fns.len {
            let d = P.fns.get(i);
            if d.is_export {
                _ = get_inst(d, ty.no_types());
            }
        }
    } else if O.unit == CG_MODULE {
        // Every non-generic function of the module (other units may call
        // any of them, directly or from a generic instance).
        for i in 0..P.fns.len {
            let d = P.fns.get(i);
            if (d.module == O.module) and (d.gparams.len == 0) and not (u.notnil(d.owner) and (d.owner.gparams.len != 0)) {
                _ = get_inst(d, ty.no_types());
            }
        }
        if g_test_mode {
            for i in 0..P.tests.len {
                if P.tests.get(i).module == O.module {
                    _ = get_inst(P.tests.get(i), ty.no_types());
                }
            }
        }
    } else {
        // The link unit: globals, error codes and names.
        for i in 0..P.globals.len {
            _ = global_ref(P.globals.get(i));
        }
        for i in 0..P.error_names.len {
            let g = L.LLVMAddGlobal(M, T_i16, u.cz(u.f1("phalange.errcode.%s", P.error_names.get(i))));
            L.LLVMSetInitializer(g, ci(T_i16, @as(u64, i) + 1));
            L.LLVMSetGlobalConstant(g, 1);
        }
        _ = error_name_fn();
        if stack_traces_on() {
            ss_globals();
        }
        _ = trap_info_global();
        if not ty.g_target.is_wasm {
            ensure_native_heap();
        }
    }
    if program_parts {
        if g_test_mode {
            emit_test_exports();
        }
        if ty.g_target.is_wasm {
            emit_trap_info_export();
        }
        emit_shadow_stack_export();
    }
    while u.notnil(pending_head) {
        let fi = pending_head;
        pending_head = fi.next;
        if u.isnil(pending_head) {
            pending_tail = u.nil.[FnInst]();
        }
        gen_fn_body(fi);
    }
    if dg.error_count() != 0 {
        return false;
    }

    let mut err = u.nil.[u8]();
    if L.LLVMVerifyModule(M, L.ReturnStatusAction, &mut err) != 0 {
        u.eprint(u.f1("phalange: internal compiler error: invalid LLVM IR generated:\n%s\n", u.from_c(err)));
        L.LLVMDisposeMessage(err);
        let dump = u.env("PHALANGE_DUMP_IR");
        if not u.isnos(dump) {
            _ = L.LLVMPrintModuleToFile(M, u.cz(dump), u.nil.[*mut u8]());
        }
        return false;
    }
    L.LLVMDisposeMessage(err);
    // At -O0 a very cheap cleanup (promote locals to SSA, merge trivial
    // blocks) makes the backend faster than skipping it.
    let mut o0_pipeline = u.env("PHALANGE_O0_PIPELINE");
    if u.isnos(o0_pipeline) {
        o0_pipeline = "function(mem2reg,simplifycfg)";
    }
    if o0_pipeline.len == 0 {
        o0_pipeline = u.nosym();
    }
    if (O.opt_level > 0) or not u.isnos(o0_pipeline) {
        let po = L.LLVMCreatePassBuilderOptions();
        let mut pipeline = o0_pipeline;
        if O.opt_level == 1 {
            pipeline = "default<O1>";
        } else if O.opt_level == 2 {
            pipeline = "default<O2>";
        } else if O.opt_level == 3 {
            pipeline = "default<O3>";
        }
        let pe = L.LLVMRunPasses(M, u.cz(pipeline), TM, po);
        L.LLVMDisposePassBuilderOptions(po);
        if u.notnil(pe) {
            let m = L.LLVMGetErrorMessage(pe);
            u.eprint(u.f1("phalange: optimization failed: %s\n", u.from_c(m)));
            L.LLVMDisposeErrorMessage(m);
            return false;
        }
    }
    if O.emit == EMIT_LLVM {
        if L.LLVMPrintModuleToFile(M, u.cz(O.out_path), &mut err) != 0 {
            u.eprint(u.f2("phalange: cannot write %s: %s\n", O.out_path, u.from_c(err)));
            return false;
        }
        return true;
    }
    let mut kind = L.ObjectFile;
    if O.emit == EMIT_ASM {
        kind = L.AssemblyFile;
    }
    if L.LLVMTargetMachineEmitToFile(TM, M, u.cz(O.out_path), kind, &mut err) != 0 {
        u.eprint(u.f1("phalange: code generation failed: %s\n", u.from_c(err)));
        L.LLVMDisposeMessage(err);
        return false;
    }
    return true;
}

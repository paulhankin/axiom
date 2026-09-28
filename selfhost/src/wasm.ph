// =============================================================================
// wasm.ph - WebAssembly code generation from the scalar IR.
//
// Control flow: the IR's (reducible) CFG becomes structured wasm with the
// dominator-tree algorithm of Ramsey, "Beyond Relooper" (ICFP 2022): each
// block is emitted once; merge points become `block`s ending just before
// them, loop headers become `loop`s, and branches become `br`s to them.
//
// Values: an instruction used once, by a later instruction of the same
// block, is emitted inline as an operand of that instruction (for effects,
// memory reads and calls only when nothing happens in between); allocas and
// constant pointer offsets are recomputed where used; everything else lives
// in a local. Phi nodes are locals written on each incoming edge, all
// inputs first, so the copies behave as one parallel assignment.
//
// Narrow integers (i1, i8, i16) are held in i32 values whose upper bits are
// unspecified, except i1, which is always 0 or 1; instructions that depend on
// them (comparisons, division, right shifts, extensions) extend first.
// =============================================================================
import "util.ph" as u;
import "ir.ph" as ir;
import "opt.ph" as opt;
import "bin.ph" as bn;
import "wobj.ph" as wo;
import "target.ph" as tg;
import "combine.ph" as cb;

// flag values
const FL_LOCAL: i32 = 0;
const FL_INLINE: i32 = 1;
const FL_REMAT: i32 = 2;

let mut UN: *mut wo.Unit = @ptr_from_int(*mut wo.Unit, 0);
let mut W64: bool = false;
let mut fsyms: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };  // Func.id -> symbol
let mut gsyms: u.List.[u32] = u.List.[u32].{ data: @ptr_from_int(*mut u32, 0), len: 0, cap: 0 };  // Global.id -> symbol
let mut sp_sym: u32 = 0;
let mut errors: bool = false;

// -----------------------------------------------------------------------------
// Per-function state
// -----------------------------------------------------------------------------
const CTX_BLOCK: i32 = 0;   // `block` whose end precedes `target`
const CTX_LOOP: i32 = 1;    // `loop` headed by `target`
const CTX_IF: i32 = 2;
const CTX_OTHER: i32 = 3;

struct Ctx {
    kind: i32,
    target: *mut ir.Block,
}

let mut F: *mut ir.Func = @ptr_from_int(*mut ir.Func, 0);
let mut C: bn.Bytes = bn.Bytes.{ b: u.List.[u8].{ data: @ptr_from_int(*mut u8, 0), len: 0, cap: 0 } };
let mut RL: u.List.[wo.Reloc] = u.List.[wo.Reloc].{ data: @ptr_from_int(*mut wo.Reloc, 0), len: 0, cap: 0 };
let mut LT: u.List.[u8] = u.List.[u8].{ data: @ptr_from_int(*mut u8, 0), len: 0, cap: 0 };  // types of locals
let mut ctx: u.List.[Ctx] = u.List.[Ctx].{ data: @ptr_from_int(*mut Ctx, 0), len: 0, cap: 0 };
let mut fp_local: u32 = 0;
let mut frame_size: u64 = 0;
let mut va_off: i64 = 0;  // frame offset of the varargs buffer
let mut nparams: u32 = 0;
let mut merge_p: *mut bool = @ptr_from_int(*mut bool, 0);
let mut header_p: *mut bool = @ptr_from_int(*mut bool, 0);
let mut nblocks: usize = 0;

fn is_merge(b: *ir.Block) bool {
    return @slice_from_ptr(merge_p, nblocks)[@int_cast(usize, b.rpo)];
}

fn is_header(b: *ir.Block) bool {
    return @slice_from_ptr(header_p, nblocks)[@int_cast(usize, b.rpo)];
}

fn vnil() *mut ir.Value {
    return u.nil.[ir.Value]();
}

fn fail(msg: []u8) void {
    u.eprint(u.f2("phalange: wasm code generation: %s (in %s)\n", msg, F.name));
    errors = true;
}

fn op(b: u8) void {
    C.u8_(b);
}

fn op_fc(n: u64) void {
    C.u8_(0xfc);
    C.uleb(n);
}

fn op_fd(n: u64) void {
    C.u8_(0xfd);
    C.uleb(n);
}

fn op_fe(n: u64) void {
    C.u8_(0xfe);
    C.uleb(n);
}

fn ptr_vt() u8 {
    if W64 {
        return wo.I64;
    }
    return wo.I32;
}

pub fn vt(t: *ir.Ty) u8 {
    let k = t.kind;
    if k == ir.T_INT {
        if t.bits > 32 {
            return wo.I64;
        }
        return wo.I32;
    }
    if k == ir.T_PTR {
        return ptr_vt();
    }
    if k == ir.T_FLOAT {
        if t.bits == 32 {
            return wo.F32;
        }
        return wo.F64;
    }
    return wo.V128;
}

fn new_local(t: u8) u32 {
    LT.push(t);
    return nparams + @int_cast(u32, LT.len - 1);
}

// local.set x followed directly by local.get x is local.tee x
let mut set_at: usize = 0;
let mut set_end: usize = 0xffff_ffff;
let mut set_idx: u32 = 0;

fn local_get(i: u32) void {
    if (C.len() == set_end) and (set_idx == i) {
        C.set_u8(set_at, 0x22);
        set_end = 0xffff_ffff;
        return;
    }
    op(0x20);
    C.uleb(@as(u64, i));
}

fn local_set(i: u32) void {
    set_at = C.len();
    op(0x21);
    C.uleb(@as(u64, i));
    set_end = C.len();
    set_idx = i;
}

fn local_tee(i: u32) void {
    op(0x22);
    C.uleb(@as(u64, i));
}

fn i32c(x: i64) void {
    op(0x41);
    C.sleb(@as(i64, @truncate(i32, x)));
}

fn i64c(x: i64) void {
    op(0x42);
    C.sleb(x);
}

fn pconst(x: u64) void {
    if W64 {
        i64c(@bit_cast(i64, x));
    } else {
        i32c(@bit_cast(i64, x));
    }
}

fn reloc(t: u8, sym: u32, addend: i64) void {
    RL.push(wo.Reloc.{ ty: t, off: @int_cast(u32, C.len()), sym: sym, addend: addend });
}

fn is64(t: *ir.Ty) bool {
    return vt(t) == wo.I64;
}

// i32 or i64 opcode by width
fn iop(t: *ir.Ty, o32: u8, o64: u8) void {
    if is64(t) {
        op(o64);
    } else {
        op(o32);
    }
}

// -----------------------------------------------------------------------------
// Narrow integers
// -----------------------------------------------------------------------------
fn bits_of(t: *ir.Ty) i32 {
    if t.kind == ir.T_PTR {
        return t.bits;
    }
    return t.bits;
}

// Zero-extends the i32 on the stack from `bits`.
fn norm_u(bits: i32) void {
    if (bits < 32) and (bits > 1) {
        i32c((@as(i64, 1) << @int_cast(u32, bits)) - 1);
        op(0x71);
    }
}

fn norm_s(bits: i32) void {
    if bits == 1 {
        // 0/1 -> 0/-1
        let t = new_local(wo.I32);
        local_set(t);
        i32c(0);
        local_get(t);
        op(0x6b);
        return;
    }
    if (bits == 8) or (bits == 16) {
        if tg.has(tg.W_SIGN_EXT) {
            op(u.pick(bits == 8, "\xc0", "\xc1")[0]);
        } else {
            let sh = @as(i64, 32 - bits);
            i32c(sh);
            op(0x74);
            i32c(sh);
            op(0x75);
        }
    }
}

// Pushes x, normalized when narrow (sign: sign-extend, else zero-extend).
fn push_ext(x: *mut ir.Value, sign: bool) void {
    push(x);
    let b = bits_of(x.ty);
    if not sign and (x.kind == ir.V_INST) and (x.op == ir.OP_LOAD) and ((b == 8) or (b == 16)) {
        return;  // loaded with load8_u / load16_u: already zero-extended
    }
    if (x.ty.kind == ir.T_INT) and (b < 32) and not ir.is_const(x) {
        if sign {
            norm_s(b);
        } else {
            norm_u(b);
        }
    } else if (x.ty.kind == ir.T_INT) and (b < 32) and sign and (x.kind == ir.V_CINT) {
        // constants are pushed zero-extended: fix the sign
        norm_s(b);
    }
}

// -----------------------------------------------------------------------------
// Operands
// -----------------------------------------------------------------------------
fn push_const(x: *mut ir.Value) void {
    let t = x.ty;
    let k = x.kind;
    if (k == ir.V_GLOBAL) {
        let s = gsyms.get(@int_cast(usize, x.gv.id));
        if x.gv.wasm_global {
            fail("address of a wasm global");
        }
        if W64 {
            op(0x42);
            reloc(wo.R_MEMORY_ADDR_SLEB64, s, 0);
            C.sleb10(0);
        } else {
            op(0x41);
            reloc(wo.R_MEMORY_ADDR_SLEB, s, 0);
            C.sleb5(0);
        }
        return;
    }
    if k == ir.V_FUNC {
        UN.uses_table = true;
        let s = fsyms.get(@int_cast(usize, x.fnp.id));
        if W64 {
            op(0x42);
            reloc(wo.R_TABLE_INDEX_SLEB64, s, 0);
            C.sleb10(0);
        } else {
            op(0x41);
            reloc(wo.R_TABLE_INDEX_SLEB, s, 0);
            C.sleb5(0);
        }
        return;
    }
    let v = vt(t);
    if v == wo.V128 {
        // v128.const with the lane bytes
        let mut bytes = bn.new_bytes();
        const_bytes(x, &mut bytes, 0);
        op_fd(12);
        C.put(bytes.view()[0..16]);
        bytes.free();
        return;
    }
    if (k == ir.V_ZERO) or (k == ir.V_UNDEF) {
        if v == wo.I32 {
            i32c(0);
        } else if v == wo.I64 {
            i64c(0);
        } else if v == wo.F32 {
            op(0x43);
            C.u32_(0);
        } else {
            op(0x44);
            C.u64_(0);
        }
        return;
    }
    if k == ir.V_CFP {
        if v == wo.F32 {
            op(0x43);
            C.u32_(@as(u64, @bit_cast(u32, @float_cast(f32, x.fv))));
        } else {
            op(0x44);
            C.u64_(@bit_cast(u64, x.fv));
        }
        return;
    }
    if v == wo.I64 {
        i64c(@bit_cast(i64, x.cv));
    } else {
        i32c(@bit_cast(i64, x.cv));
    }
}

// Little-endian bytes of a constant (for v128.const and data).
fn const_bytes(x: *mut ir.Value, out: *mut bn.Bytes, base: usize) void {
    let t = x.ty;
    while out.len() < (base + @truncate(usize, t.size)) {
        out.u8_(0);
    }
    let k = x.kind;
    if (k == ir.V_ZERO) or (k == ir.V_UNDEF) {
        return;
    }
    if k == ir.V_CINT {
        for i in 0..@truncate(usize, t.size) {
            out.set_u8(base + i, @truncate(u8, x.cv >> @int_cast(u32, i * 8)));
        }
        return;
    }
    if k == ir.V_CFP {
        if t.bits == 32 {
            out.set_u32(base, @as(u64, @bit_cast(u32, @float_cast(f32, x.fv))));
        } else {
            out.set_u64(base, @bit_cast(u64, x.fv));
        }
        return;
    }
    if k == ir.V_CAGG {
        let mut esz: u64 = 0;
        if (t.kind == ir.T_VEC) or (t.kind == ir.T_ARRAY) {
            esz = t.elem.size;
        }
        for i in 0..x.xs.len {
            let mut off: u64 = 0;
            if t.kind == ir.T_STRUCT {
                off = t.offs[i];
            } else {
                off = esz * @as(u64, i);
            }
            const_bytes(x.xs.get(i), out, base + @truncate(usize, off));
        }
    }
}

fn is_remat(x: *ir.Value) bool {
    return (x.kind == ir.V_INST) and (x.flag == FL_REMAT);
}

pub fn push(x: *mut ir.Value) void {
    let k = x.kind;
    if k == ir.V_PARAM {
        local_get(@truncate(u32, x.cv));
        return;
    }
    if k != ir.V_INST {
        push_const(x);
        return;
    }
    if (x.flag == FL_INLINE) or (x.flag == FL_REMAT) {
        expr(x);
        return;
    }
    if x.loc < 0 {
        fail(u.f1("value used without a local: %s", ir.inst_str(x)));
        return;
    }
    local_get(@int_cast(u32, x.loc));
}

// Pushes the base of an address and returns the constant offset to put in
// the memory instruction.
fn push_addr(p: *mut ir.Value) u64 {
    if (p.kind == ir.V_INST) and (p.op == ir.OP_ALLOCA) {
        local_get(fp_local);
        return @bit_cast(u64, p.aux);
    }
    if (p.kind == ir.V_INST) and (p.op == ir.OP_PTRADD) and ((p.flag == FL_INLINE) or (p.flag == FL_REMAT)) and
       (p.b.kind == ir.V_CINT) and (ir.sval(p.b) >= 0) and (ir.sval(p.b) < 0x4000_0000) {
        let base = push_addr(p.a);
        return base + p.b.cv;
    }
    push(p);
    return 0;
}

fn memarg(align: u32, size: u64, off: u64) void {
    let mut a = @as(u64, align);
    if (a == 0) or (a > size) {
        a = size;
    }
    let mut lg: u64 = 0;
    while (@as(u64, 1) << @truncate(u32, lg)) < a {
        lg += 1;
    }
    C.uleb(lg);
    C.uleb(off);
}

// -----------------------------------------------------------------------------
// Instructions
// -----------------------------------------------------------------------------
fn lane_shape(t: *ir.Ty) i32 {
    // 0: i8x16, 1: i16x8, 2: i32x4, 3: i64x2, 4: f32x4, 5: f64x2
    let e = t.elem;
    if e.kind == ir.T_FLOAT {
        if e.bits == 32 {
            return 4;
        }
        return 5;
    }
    if e.bits == 8 {
        return 0;
    }
    if e.bits == 16 {
        return 1;
    }
    if e.bits == 32 {
        return 2;
    }
    return 3;
}

fn vec_binop(o: i32, t: *ir.Ty) void {
    let sh = lane_shape(t);
    if (o == ir.OP_AND) or (o == ir.OP_OR) or (o == ir.OP_XOR) {
        if o == ir.OP_AND {
            op_fd(78);
        } else if o == ir.OP_OR {
            op_fd(80);
        } else {
            op_fd(81);
        }
        return;
    }
    let adds: [4]u64 = [110, 142, 174, 206];
    let subs: [4]u64 = [113, 145, 177, 209];
    let muls: [4]u64 = [0, 149, 181, 213];
    if sh < 4 {
        let i = @int_cast(usize, sh);
        if o == ir.OP_ADD {
            op_fd(@as(u64, adds[i]));
            return;
        }
        if o == ir.OP_SUB {
            op_fd(@as(u64, subs[i]));
            return;
        }
        if (o == ir.OP_MUL) and (sh > 0) {
            op_fd(@as(u64, muls[i]));
            return;
        }
    } else {
        let mut base: u64 = 228;  // f32x4.add
        if sh == 5 {
            base = 240;  // f64x2.add
        }
        if o == ir.OP_FADD {
            op_fd(base);
            return;
        }
        if o == ir.OP_FSUB {
            op_fd(base + 1);
            return;
        }
        if o == ir.OP_FMUL {
            op_fd(base + 2);
            return;
        }
        if o == ir.OP_FDIV {
            op_fd(base + 3);
            return;
        }
    }
    fail(u.f1("vector operation %s", u.idec(o)));
}

// Shifts of vectors by a lane-uniform count (the IR's second operand is a
// splat of the scalar count, or a vector whose lane 0 we use).
fn vec_shift(v: *mut ir.Value) void {
    let sh = lane_shape(v.ty);
    push(v.a);
    let c = v.b;
    if (c.kind == ir.V_INST) and (c.op == ir.OP_SPLAT) {
        push(c.a);
        if is64(c.a.ty) {
            op(0xa7);
        }
    } else {
        push(c);
        extract_lane(c.ty, 0, false);
        if is64(c.ty.elem) {
            op(0xa7);
        }
    }
    let base: [4]u64 = [107, 139, 171, 203];
    let mut k: u64 = 0;
    if v.op == ir.OP_ASHR {
        k = 1;
    } else if v.op == ir.OP_LSHR {
        k = 2;
    }
    op_fd(@as(u64, base[@int_cast(usize, sh)]) + k);
}

fn extract_lane(t: *ir.Ty, lane: u64, signed: bool) void {
    let sh = lane_shape(t);
    if sh == 0 {
        op_fd(@as(u64, u.pick(signed, "\x15", "\x16")[0]));
    } else if sh == 1 {
        op_fd(@as(u64, u.pick(signed, "\x18", "\x19")[0]));
    } else {
        let codes: [6]u64 = [0, 0, 27, 29, 31, 33];
        op_fd(@as(u64, codes[@int_cast(usize, sh)]));
    }
    C.u8_(@truncate(u8, lane));
}

fn replace_lane(t: *ir.Ty, lane: u64) void {
    let codes: [6]u64 = [23, 26, 28, 30, 32, 34];
    op_fd(@as(u64, codes[@int_cast(usize, lane_shape(t))]));
    C.u8_(@truncate(u8, lane));
}

fn splat_op(t: *ir.Ty) void {
    op_fd(15 + @as(u64, @bit_cast(u32, lane_shape(t))));
}

// i32 comparison opcodes by predicate
fn icmp_op(p: i32, wide: bool) void {
    let mut base: u8 = 0x46;
    if wide {
        base = 0x51;
    }
    let mut k: u8 = 0;
    if p == ir.IntEQ {
        k = 0;
    } else if p == ir.IntNE {
        k = 1;
    } else if p == ir.IntSLT {
        k = 2;
    } else if p == ir.IntULT {
        k = 3;
    } else if p == ir.IntSGT {
        k = 4;
    } else if p == ir.IntUGT {
        k = 5;
    } else if p == ir.IntSLE {
        k = 6;
    } else if p == ir.IntULE {
        k = 7;
    } else if p == ir.IntSGE {
        k = 8;
    } else {
        k = 9;
    }
    op(base + k);
}

fn is_signed_pred(p: i32) bool {
    return (p == ir.IntSLT) or (p == ir.IntSGT) or (p == ir.IntSLE) or (p == ir.IntSGE);
}

fn vec_icmp(p: i32, t: *ir.Ty) void {
    let sh = lane_shape(t);
    if sh == 3 {
        // i64x2: eq ne lt_s gt_s le_s ge_s only; unsigned via flipped sign bits
        let mut k: u64 = 0;
        if p == ir.IntEQ {
            k = 0;
        } else if p == ir.IntNE {
            k = 1;
        } else if (p == ir.IntSLT) or (p == ir.IntULT) {
            k = 2;
        } else if (p == ir.IntSGT) or (p == ir.IntUGT) {
            k = 3;
        } else if (p == ir.IntSLE) or (p == ir.IntULE) {
            k = 4;
        } else {
            k = 5;
        }
        op_fd(214 + k);
        return;
    }
    let base: [3]u64 = [35, 45, 55];
    let mut k: u64 = 0;
    if p == ir.IntEQ {
        k = 0;
    } else if p == ir.IntNE {
        k = 1;
    } else if p == ir.IntSLT {
        k = 2;
    } else if p == ir.IntULT {
        k = 3;
    } else if p == ir.IntSGT {
        k = 4;
    } else if p == ir.IntUGT {
        k = 5;
    } else if p == ir.IntSLE {
        k = 6;
    } else if p == ir.IntULE {
        k = 7;
    } else if p == ir.IntSGE {
        k = 8;
    } else {
        k = 9;
    }
    op_fd(@as(u64, base[@int_cast(usize, sh)]) + k);
}

fn fcmp_k(p: i32) u64 {
    if p == ir.RealOEQ {
        return 0;
    }
    if p == ir.RealUNE {
        return 1;
    }
    if p == ir.RealOLT {
        return 2;
    }
    if p == ir.RealOGT {
        return 3;
    }
    if p == ir.RealOLE {
        return 4;
    }
    return 5;
}

// Flips the sign bit of each i64 lane of the v128 on the stack.
fn flip_i64_signs() void {
    op_fd(12);
    for i in @as(usize, 0)..2 {
        C.u64_(0x8000_0000_0000_0000);
    }
    op_fd(81);
}

fn gen_icmp(v: *mut ir.Value) void {
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let t = v.a.ty;
    if t.kind == ir.T_VEC {
        let sh = lane_shape(t);
        let unsigned64 = (sh == 3) and ((p == ir.IntULT) or (p == ir.IntUGT) or (p == ir.IntULE) or (p == ir.IntUGE));
        push(v.a);
        if unsigned64 {
            flip_i64_signs();
        }
        push(v.b);
        if unsigned64 {
            flip_i64_signs();
        }
        vec_icmp(p, t);
        return;
    }
    let sg = is_signed_pred(p);
    push_ext(v.a, sg);
    push_ext(v.b, sg);
    icmp_op(p, is64(t));
}

fn gen_fcmp(v: *mut ir.Value) void {
    let p = @truncate(i32, @bit_cast(i64, v.cv));
    let t = v.a.ty;
    push(v.a);
    push(v.b);
    let k = fcmp_k(p);
    if t.kind == ir.T_VEC {
        if lane_shape(t) == 4 {
            op_fd(65 + k);
        } else {
            op_fd(71 + k);
        }
        return;
    }
    if t.bits == 32 {
        op(@truncate(u8, 0x5b + k));
    } else {
        op(@truncate(u8, 0x61 + k));
    }
}

fn gen_binop(v: *mut ir.Value) void {
    let o = v.op;
    let t = v.ty;
    if t.kind == ir.T_VEC {
        if (o == ir.OP_SHL) or (o == ir.OP_LSHR) or (o == ir.OP_ASHR) {
            vec_shift(v);
            return;
        }
        push(v.a);
        push(v.b);
        vec_binop(o, t);
        return;
    }
    if t.kind == ir.T_FLOAT {
        push(v.a);
        push(v.b);
        let mut base: u8 = 0x92;
        if t.bits == 64 {
            base = 0xa0;
        }
        if o == ir.OP_FADD {
            op(base);
        } else if o == ir.OP_FSUB {
            op(base + 1);
        } else if o == ir.OP_FMUL {
            op(base + 2);
        } else if o == ir.OP_FDIV {
            op(base + 3);
        } else {
            fail("frem is not supported on wasm yet");
        }
        return;
    }
    let bits = t.bits;
    let w = is64(t);
    if (o == ir.OP_UDIV) or (o == ir.OP_UREM) or (o == ir.OP_LSHR) {
        push_ext(v.a, false);
        if o == ir.OP_LSHR {
            push(v.b);
            if bits < 32 {
                norm_u(bits);
            }
        } else {
            push_ext(v.b, false);
        }
    } else if (o == ir.OP_SDIV) or (o == ir.OP_SREM) or (o == ir.OP_ASHR) {
        push_ext(v.a, true);
        if o == ir.OP_ASHR {
            push(v.b);
            if bits < 32 {
                norm_u(bits);
            }
        } else {
            push_ext(v.b, true);
        }
    } else {
        push(v.a);
        push(v.b);
        if (o == ir.OP_SHL) and (bits < 32) {
            norm_u(bits);
        }
    }
    let mut k: u8 = 0;
    if o == ir.OP_ADD {
        k = 3;
    } else if o == ir.OP_SUB {
        k = 4;
    } else if o == ir.OP_MUL {
        k = 5;
    } else if o == ir.OP_SDIV {
        k = 6;
    } else if o == ir.OP_UDIV {
        k = 7;
    } else if o == ir.OP_SREM {
        k = 8;
    } else if o == ir.OP_UREM {
        k = 9;
    } else if o == ir.OP_AND {
        k = 10;
    } else if o == ir.OP_OR {
        k = 11;
    } else if o == ir.OP_XOR {
        k = 12;
    } else if o == ir.OP_SHL {
        k = 13;
    } else if o == ir.OP_ASHR {
        k = 14;
    } else {
        k = 15;
    }
    if w {
        op(0x79 + k);
    } else {
        op(0x67 + k);
    }
    if (bits == 1) and ((o == ir.OP_ADD) or (o == ir.OP_SUB) or (o == ir.OP_MUL) or (o == ir.OP_SHL)) {
        i32c(1);
        op(0x71);
    }
}

fn gen_cast(v: *mut ir.Value) void {
    let o = v.op;
    let from = v.a.ty;
    let to = v.ty;
    if o == ir.OP_ZEXT {
        push_ext(v.a, false);
        if is64(to) and not is64(from) {
            op(0xad);
        }
        return;
    }
    if o == ir.OP_SEXT {
        if to.kind == ir.T_VEC {
            push(v.a);  // lane masks are already all-ones / zeros
            return;
        }
        push_ext(v.a, true);
        if is64(to) and not is64(from) {
            op(0xac);
        }
        return;
    }
    if o == ir.OP_TRUNC {
        push(v.a);
        if is64(from) and not is64(to) {
            op(0xa7);
        }
        if to.bits == 1 {
            i32c(1);
            op(0x71);
        }
        return;
    }
    if (o == ir.OP_PTRTOINT) or (o == ir.OP_INTTOPTR) {
        push(v.a);
        return;
    }
    if o == ir.OP_BITCAST {
        push(v.a);
        let fv = vt(from);
        let tv = vt(to);
        if fv == tv {
            return;
        }
        if (fv == wo.F32) and (tv == wo.I32) {
            op(0xbc);
        } else if (fv == wo.F64) and (tv == wo.I64) {
            op(0xbd);
        } else if (fv == wo.I32) and (tv == wo.F32) {
            op(0xbe);
        } else if (fv == wo.I64) and (tv == wo.F64) {
            op(0xbf);
        } else {
            fail("unsupported bitcast");
        }
        return;
    }
    if o == ir.OP_FPEXT {
        push(v.a);
        if to.kind == ir.T_VEC {
            op_fd(95);
        } else {
            op(0xbb);
        }
        return;
    }
    if o == ir.OP_FPTRUNC {
        push(v.a);
        op(0xb6);
        return;
    }
    if (o == ir.OP_SITOFP) or (o == ir.OP_UITOFP) {
        let sg = o == ir.OP_SITOFP;
        push_ext(v.a, sg);
        let w = is64(from);
        let mut code: u8 = 0;
        if to.bits == 32 {
            code = 0xb2;
        } else {
            code = 0xb7;
        }
        if w {
            code += 2;
        }
        if not sg {
            code += 1;
        }
        op(code);
        return;
    }
    if (o == ir.OP_FPTOSI_SAT) or (o == ir.OP_FPTOUI_SAT) {
        let sg = o == ir.OP_FPTOSI_SAT;
        push(v.a);
        let from64 = from.bits == 64;
        let wide = is64(to);
        if tg.has(tg.W_NONTRAPPING_FPTOINT) {
            let mut n: u64 = 0;
            if wide {
                n = 4;
            }
            if from64 {
                n += 2;
            }
            if not sg {
                n += 1;
            }
            op_fc(n);
        } else {
            trunc_sat_fallback(from64, wide, sg);
        }
        if to.bits < 32 {
            clamp_narrow(to.bits, sg);
        }
        return;
    }
    fail(u.f1("cast %s", u.idec(o)));
}

// The i32 on the stack clamped to the range of a narrower integer type (it
// holds a saturated i32 / u32 conversion result).
fn clamp_narrow(bits: i32, sg: bool) void {
    let t = new_local(wo.I32);
    local_set(t);
    let mut lo: i64 = 0;
    let mut hi: i64 = (@as(i64, 1) << @int_cast(u32, bits)) - 1;
    if sg {
        lo = -(@as(i64, 1) << @int_cast(u32, bits - 1));
        hi = (@as(i64, 1) << @int_cast(u32, bits - 1)) - 1;
    }
    // t = x > hi ? hi : x
    i32c(hi);
    local_get(t);
    local_get(t);
    i32c(hi);
    if sg {
        op(0x4a);  // gt_s
    } else {
        op(0x4b);  // gt_u
    }
    op(0x1b);
    if sg {
        // t < lo ? lo : t
        local_set(t);
        i32c(lo);
        local_get(t);
        local_get(t);
        i32c(lo);
        op(0x48);  // lt_s
        op(0x1b);
    }
}

// Saturating float -> int conversion without the nontrapping-fptoint
// extension: NaN gives 0, out-of-range values the nearest bound.
fn trunc_sat_fallback(from64: bool, wide: bool, sg: bool) void {
    let fvt = u.pick(from64, "\x7c", "\x7d")[0];
    let x = new_local(fvt);
    local_set(x);
    let rv = u.pick(wide, "\x7e", "\x7f")[0];
    // limits as floats: [lo, hi) is safe for the trapping conversion
    let mut lo: f64 = 0.0;
    let mut hi: f64 = 0.0;
    if wide {
        if sg {
            lo = -9223372036854775808.0;
            hi = 9223372036854775808.0;
        } else {
            lo = 0.0;
            hi = 18446744073709551616.0;
        }
    } else {
        if sg {
            lo = -2147483648.0;
            hi = 2147483648.0;
        } else {
            lo = 0.0;
            hi = 4294967296.0;
        }
    }
    let r = new_local(rv);
    // if x != x: 0 else if x < lo: min else if x >= hi: max else trunc(x)
    local_get(x);
    local_get(x);
    op(u.pick(from64, "\x62", "\x5c")[0]);  // ne
    op(0x04);
    op(rv);
    if wide {
        i64c(0);
    } else {
        i32c(0);
    }
    op(0x05);
    local_get(x);
    fconst(from64, lo);
    op(u.pick(from64, "\x63", "\x5d")[0]);  // lt
    op(0x04);
    op(rv);
    if wide {
        if sg {
            i64c(-9223372036854775807 - 1);
        } else {
            i64c(0);
        }
    } else {
        if sg {
            i32c(-2147483648);
        } else {
            i32c(0);
        }
    }
    op(0x05);
    local_get(x);
    fconst(from64, hi);
    op(u.pick(from64, "\x66", "\x60")[0]);  // ge
    op(0x04);
    op(rv);
    if wide {
        if sg {
            i64c(9223372036854775807);
        } else {
            i64c(-1);
        }
    } else {
        if sg {
            i32c(2147483647);
        } else {
            i32c(-1);
        }
    }
    op(0x05);
    local_get(x);
    let mut code: u8 = 0xa8;
    if wide {
        code = 0xae;
    }
    if from64 {
        code += 2;
    }
    if not sg {
        code += 1;
    }
    op(code);
    op(0x0b);
    op(0x0b);
    op(0x0b);
    local_set(r);
    local_get(r);
}

fn fconst(is64f: bool, x: f64) void {
    if is64f {
        op(0x44);
        C.u64_(@bit_cast(u64, x));
    } else {
        op(0x43);
        C.u32_(@as(u64, @bit_cast(u32, @float_cast(f32, x))));
    }
}

fn load_op(t: *ir.Ty, atomic: bool) void {
    let k = t.kind;
    if atomic {
        if (k == ir.T_INT) or (k == ir.T_PTR) {
            let b = t.bits;
            if is64(t) {
                op_fe(0x11);
            } else if b == 8 {
                op_fe(0x12);
            } else if b == 16 {
                op_fe(0x13);
            } else {
                op_fe(0x10);
            }
            return;
        }
    }
    if k == ir.T_VEC {
        op_fd(0);
        return;
    }
    if k == ir.T_FLOAT {
        op(u.pick(t.bits == 32, "\x2a", "\x2b")[0]);
        return;
    }
    if is64(t) {
        op(0x29);
        return;
    }
    let b = t.bits;
    if (b == 8) or (b == 1) {
        op(0x2d);
    } else if b == 16 {
        op(0x2f);
    } else {
        op(0x28);
    }
}

fn store_op(t: *ir.Ty, atomic: bool) void {
    let k = t.kind;
    if atomic {
        if (k == ir.T_INT) or (k == ir.T_PTR) {
            let b = t.bits;
            if is64(t) {
                op_fe(0x18);
            } else if b == 8 {
                op_fe(0x19);
            } else if b == 16 {
                op_fe(0x1a);
            } else {
                op_fe(0x17);
            }
            return;
        }
    }
    if k == ir.T_VEC {
        op_fd(11);
        return;
    }
    if k == ir.T_FLOAT {
        op(u.pick(t.bits == 32, "\x38", "\x39")[0]);
        return;
    }
    if is64(t) {
        op(0x37);
        return;
    }
    let b = t.bits;
    if (b == 8) or (b == 1) {
        op(0x3a);
    } else if b == 16 {
        op(0x3b);
    } else {
        op(0x36);
    }
}

fn is_wasm_global(p: *ir.Value) bool {
    return (p.kind == ir.V_GLOBAL) and p.gv.wasm_global;
}

fn gen_load(v: *mut ir.Value) void {
    if is_wasm_global(v.a) {
        op(0x23);
        reloc(wo.R_GLOBAL_INDEX_LEB, gsyms.get(@int_cast(usize, v.a.gv.id)), 0);
        C.uleb5(0);
        return;
    }
    let off = push_addr(v.a);
    let atomic = (v.flags & ir.F_ATOMIC) != 0;
    load_op(v.ty, atomic and tg.has(tg.W_ATOMICS));
    let mut al = v.align;
    if atomic {
        al = @int_cast(u32, v.ty.size);
    }
    memarg(al, v.ty.size, off);
}

fn gen_store(v: *mut ir.Value) void {
    if is_wasm_global(v.b) {
        push(v.a);
        op(0x24);
        reloc(wo.R_GLOBAL_INDEX_LEB, gsyms.get(@int_cast(usize, v.b.gv.id)), 0);
        C.uleb5(0);
        return;
    }
    let off = push_addr(v.b);
    push(v.a);
    let atomic = (v.flags & ir.F_ATOMIC) != 0;
    let t = v.a.ty;
    store_op(t, atomic and tg.has(tg.W_ATOMICS));
    let mut al = v.align;
    if atomic {
        al = @int_cast(u32, t.size);
    }
    memarg(al, t.size, off);
}

// Atomic read-modify-write opcode: base + width variant.
fn rmw_code(base: u64, t: *ir.Ty) u64 {
    if is64(t) {
        return base + 1;
    }
    if t.bits == 8 {
        return base + 2;
    }
    if t.bits == 16 {
        return base + 3;
    }
    return base;
}

fn gen_rmw(v: *mut ir.Value) void {
    let o = @truncate(i32, @bit_cast(i64, v.cv));
    let t = v.ty;
    let sz = t.size;
    if (o == ir.RMW_MIN) or (o == ir.RMW_MAX) or (o == ir.RMW_UMIN) or (o == ir.RMW_UMAX) or
       not tg.has(tg.W_ATOMICS) {
        rmw_loop(v, o);
        return;
    }
    let off = push_addr(v.a);
    push(v.b);
    let mut base: u64 = 0x1e;
    if o == ir.RMW_SUB {
        base = 0x25;
    } else if o == ir.RMW_AND {
        base = 0x2c;
    } else if o == ir.RMW_OR {
        base = 0x33;
    } else if o == ir.RMW_XOR {
        base = 0x3a;
    } else if o == ir.RMW_XCHG {
        base = 0x41;
    }
    op_fe(rmw_code(base, t));
    memarg(@int_cast(u32, sz), sz, off);
}

// old = *p; loop { new = op(old, x); cur = cmpxchg(p, old, new); if cur == old break; old = cur }
fn rmw_loop(v: *mut ir.Value, o: i32) void {
    let t = v.ty;
    let w = vt(t);
    let sz = t.size;
    let pl = new_local(ptr_vt());
    let xl = new_local(w);
    let old = new_local(w);
    let cur = new_local(w);
    push(v.a);
    local_set(pl);
    push(v.b);
    local_set(xl);
    let atomics = tg.has(tg.W_ATOMICS);
    local_get(pl);
    load_op(t, atomics);
    memarg(@int_cast(u32, sz), sz, 0);
    local_set(old);
    op(0x03);
    op(0x40);
    local_get(pl);
    local_get(old);
    // new value
    let sg = (o == ir.RMW_MIN) or (o == ir.RMW_MAX);
    if (o == ir.RMW_MIN) or (o == ir.RMW_MAX) or (o == ir.RMW_UMIN) or (o == ir.RMW_UMAX) {
        let is_min = (o == ir.RMW_MIN) or (o == ir.RMW_UMIN);
        // select(old, x, old < x) for min, old > x for max
        local_get(old);
        local_get(xl);
        local_get(old);
        if t.bits < 32 {
            if sg {
                norm_s(t.bits);
            } else {
                norm_u(t.bits);
            }
        }
        local_get(xl);
        if t.bits < 32 {
            if sg {
                norm_s(t.bits);
            } else {
                norm_u(t.bits);
            }
        }
        if is_min {
            if sg {
                icmp_op(ir.IntSLT, is64(t));
            } else {
                icmp_op(ir.IntULT, is64(t));
            }
        } else {
            if sg {
                icmp_op(ir.IntSGT, is64(t));
            } else {
                icmp_op(ir.IntUGT, is64(t));
            }
        }
        op(0x1b);
    } else {
        local_get(old);
        local_get(xl);
        if o == ir.RMW_ADD {
            iop(t, 0x6a, 0x7c);
        } else if o == ir.RMW_SUB {
            iop(t, 0x6b, 0x7d);
        } else if o == ir.RMW_AND {
            iop(t, 0x71, 0x83);
        } else if o == ir.RMW_OR {
            iop(t, 0x72, 0x84);
        } else if o == ir.RMW_XOR {
            iop(t, 0x73, 0x85);
        } else {
            op(0x1a);
            op(0x1a);
            local_get(xl);
        }
    }
    if atomics {
        op_fe(rmw_code(0x48, t));
        memarg(@int_cast(u32, sz), sz, 0);
    } else {
        // plain store, then the loaded value compares equal
        let nv = new_local(w);
        local_set(nv);
        local_set(cur);
        op(0x1a);
        local_get(pl);
        local_get(nv);
        store_op(t, false);
        memarg(@int_cast(u32, sz), sz, 0);
        local_get(old);
    }
    local_tee(cur);
    local_get(old);
    if t.bits < 32 {
        // compare the accessed bits only
        let tmp = new_local(wo.I32);
        local_set(tmp);
        norm_u(t.bits);
        local_get(tmp);
        norm_u(t.bits);
    }
    icmp_op(ir.IntNE, is64(t));
    op(0x04);
    op(0x40);
    local_get(cur);
    local_set(old);
    op(0x0c);
    C.uleb(1);
    op(0x0b);
    op(0x0b);
    local_get(old);
}

fn gen_cmpxchg(v: *mut ir.Value) void {
    let t = v.ty;
    let sz = t.size;
    if not tg.has(tg.W_ATOMICS) {
        // single-threaded: load, compare, maybe store
        let w = vt(t);
        let pl = new_local(ptr_vt());
        let old = new_local(w);
        push(v.a);
        local_tee(pl);
        load_op(t, false);
        memarg(@int_cast(u32, sz), sz, 0);
        local_set(old);
        local_get(old);
        push_ext(v.b, false);
        if t.bits < 32 {
            let tmp = new_local(wo.I32);
            local_set(tmp);
            norm_u(t.bits);
            local_get(tmp);
        }
        icmp_op(ir.IntEQ, is64(t));
        op(0x04);
        op(0x40);
        local_get(pl);
        push(v.c);
        store_op(t, false);
        memarg(@int_cast(u32, sz), sz, 0);
        op(0x0b);
        local_get(old);
        return;
    }
    let off = push_addr(v.a);
    push_ext(v.b, false);
    push(v.c);
    op_fe(rmw_code(0x48, t));
    memarg(@int_cast(u32, sz), sz, off);
}

fn gen_memop(v: *mut ir.Value) void {
    let o = v.op;
    if (o == ir.OP_MEMCPY) and (v.c.kind == ir.V_CINT) and (v.c.cv <= 64) {
        // small copies: loads and stores
        let n = v.c.cv;
        let d = new_local(ptr_vt());
        let s = new_local(ptr_vt());
        let doff = push_addr(v.a);
        local_set(d);
        let soff = push_addr(v.b);
        local_set(s);
        let mut k: u64 = 0;
        while k < n {
            let mut w: u64 = 8;
            while (k + w) > n {
                w = w / 2;
            }
            local_get(d);
            local_get(s);
            if w == 8 {
                op(0x29);
            } else if w == 4 {
                op(0x28);
            } else if w == 2 {
                op(0x2f);
            } else {
                op(0x2d);
            }
            memarg(1, w, soff + k);
            if w == 8 {
                op(0x37);
            } else if w == 4 {
                op(0x36);
            } else if w == 2 {
                op(0x3b);
            } else {
                op(0x3a);
            }
            memarg(1, w, doff + k);
            k += w;
        }
        return;
    }
    if not tg.has(tg.W_BULK_MEMORY) {
        mem_loop(v);
        return;
    }
    push(v.a);
    push(v.b);
    push(v.c);
    if o == ir.OP_MEMSET {
        op_fc(11);
        op(0);
    } else {
        op_fc(10);
        op(0);
        op(0);
    }
}

// memcpy / memmove / memset as a byte loop (no bulk-memory extension).
fn mem_loop(v: *mut ir.Value) void {
    let pv = ptr_vt();
    let d = new_local(pv);
    let s = new_local(pv);
    let n = new_local(pv);
    let i = new_local(pv);
    push(v.a);
    local_set(d);
    push(v.b);
    local_set(s);
    push(v.c);
    local_set(n);
    let is_set = v.op == ir.OP_MEMSET;
    // memmove: copy backwards when d > s
    let back = new_local(wo.I32);
    if v.op == ir.OP_MEMMOVE {
        local_get(d);
        local_get(s);
        if W64 {
            op(0x56);
        } else {
            op(0x4b);
        }
    } else {
        i32c(0);
    }
    local_set(back);
    pconst(0);
    local_set(i);
    op(0x02);
    op(0x40);
    op(0x03);
    op(0x40);
    local_get(i);
    local_get(n);
    if W64 {
        op(0x5a);  // i64.ge_u
    } else {
        op(0x4f);
    }
    op(0x0d);
    C.uleb(1);
    // index j = back ? n-1-i : i
    let j = new_local(pv);
    local_get(n);
    pconst(1);
    padd_sub(true);
    local_get(i);
    padd_sub(true);
    local_get(i);
    local_get(back);
    op(0x1b);
    local_set(j);
    local_get(d);
    local_get(j);
    padd_sub(false);
    if is_set {
        local_get(s);
    } else {
        local_get(s);
        local_get(j);
        padd_sub(false);
        op(0x2d);
        memarg(1, 1, 0);
    }
    op(0x3a);
    memarg(1, 1, 0);
    local_get(i);
    pconst(1);
    padd_sub(false);
    local_set(i);
    op(0x0c);
    C.uleb(0);
    op(0x0b);
    op(0x0b);
}

fn padd_sub(sub: bool) void {
    if W64 {
        op(u.pick(sub, "\x7d", "\x7c")[0]);
    } else {
        op(u.pick(sub, "\x6b", "\x6a")[0]);
    }
}

fn arg_ext(x: *mut ir.Value, e: u8) void {
    if e == 1 {
        push_ext(x, false);
    } else if e == 2 {
        push_ext(x, true);
    } else {
        push(x);
    }
}

// Size and alignment of a C variadic argument in the wasm varargs buffer.
fn va_size(t: *ir.Ty) u64 {
    if t.size <= 4 {
        return 4;
    }
    return t.size;
}

fn va_buffer_size(v: *ir.Value) u64 {
    let nfixed = v.fty.params.len;
    let mut off: u64 = 0;
    for i in nfixed..v.xs.len {
        let sz = va_size(v.xs.get(i).ty);
        off = (off + sz - 1) & ~(sz - 1);
        off += sz;
    }
    return off;
}

fn gen_call(v: *mut ir.Value) void {
    if v.fty.vararg {
        // C variadic call (wasm C ABI): the extra arguments go in a buffer on
        // the stack, whose address is passed after the fixed arguments
        let nfixed = v.fty.params.len;
        for i in 0..nfixed {
            let mut e: u8 = 0;
            if (i + 1) < v.ext.len {
                e = v.ext[i + 1];
            }
            arg_ext(v.xs.get(i), e);
        }
        let mut off: u64 = 0;
        for i in nfixed..v.xs.len {
            let x = v.xs.get(i);
            let sz = va_size(x.ty);
            off = (off + sz - 1) & ~(sz - 1);
            local_get(fp_local);
            push_ext(x, true);
            if (x.ty.kind == ir.T_INT) and (x.ty.size < 4) {
                store_op(ir.t_int(32), false);
            } else {
                store_op(x.ty, false);
            }
            memarg(@int_cast(u32, sz), sz, @bit_cast(u64, va_off) + off);
            off += sz;
        }
        local_get(fp_local);
        pconst(@bit_cast(u64, va_off));
        padd_sub(false);
        op(0x10);
        reloc(wo.R_FUNCTION_INDEX_LEB, fsyms.get(@int_cast(usize, v.a.fnp.id)), 0);
        C.uleb5(0);
        return;
    }
    for i in 0..v.xs.len {
        let mut e: u8 = 0;
        if (i + 1) < v.ext.len {
            e = v.ext[i + 1];
        }
        arg_ext(v.xs.get(i), e);
    }
    let cal = v.a;
    if cal.kind == ir.V_FUNC {
        op(0x10);
        reloc(wo.R_FUNCTION_INDEX_LEB, fsyms.get(@int_cast(usize, cal.fnp.id)), 0);
        C.uleb5(0);
        return;
    }
    push(cal);
    if W64 {
        op(0xa7);
    }
    let ti = fn_type(v.fty);
    op(0x11);
    reloc(wo.R_TYPE_INDEX_LEB, ti, 0);
    C.uleb5(@as(u64, ti));
    C.u8_(0);
    UN.uses_table = true;
}

pub fn fn_type(t: *ir.Ty) u32 {
    let ps = u.newn.[u8](t.params.len + 2);
    for i in 0..t.params.len {
        ps[i] = vt(t.params[i]);
    }
    let mut np = t.params.len;
    if t.vararg {
        ps[np] = ptr_vt();
        np += 1;
    }
    let mut rs: []u8 = "";
    if t.ret.kind != ir.T_VOID {
        let r = u.newn.[u8](1);
        r[0] = vt(t.ret);
        rs = r[0..1];
    }
    return wo.type_index(UN, ps[0..np], rs);
}

fn gen_intr(v: *mut ir.Value) void {
    let id = @truncate(i32, @bit_cast(i64, v.cv));
    let t = v.ty;
    let n = v.xs.len;
    if id == ir.IN_TRAP {
        op(0x00);
        return;
    }
    if id == ir.IN_MEMSIZE {
        op(0x3f);
        op(0);
        return;
    }
    if id == ir.IN_MEMGROW {
        push(v.xs.get(0));
        op(0x40);
        op(0);
        return;
    }
    if id == ir.IN_WAIT32 {
        push(v.xs.get(0));
        push(v.xs.get(1));
        push(v.xs.get(2));
        op_fe(1);
        memarg(4, 4, 0);
        return;
    }
    if id == ir.IN_NOTIFY {
        push(v.xs.get(0));
        push(v.xs.get(1));
        op_fe(0);
        memarg(4, 4, 0);
        return;
    }
    if id == ir.IN_SET_SP {
        push(v.xs.get(0));
        op(0x24);
        reloc(wo.R_GLOBAL_INDEX_LEB, sp_sym, 0);
        C.uleb5(0);
        return;
    }
    let a = v.xs.get(0);
    if t.kind == ir.T_VEC {
        gen_vec_intr(v, id);
        return;
    }
    if (id == ir.IN_ANYTRUE) or (id == ir.IN_ALLTRUE) or (id == ir.IN_BITMASK) or (id == ir.IN_SWIZZLE) or
       (id == ir.IN_DOT_I16) {
        gen_vec_intr(v, id);
        return;
    }
    if t.kind == ir.T_FLOAT {
        let f64_ = t.bits == 64;
        let mut base: u8 = 0x8b;
        if f64_ {
            base = 0x99;
        }
        if id == ir.IN_FABS {
            push(a);
            op(base);
        } else if id == ir.IN_CEIL {
            push(a);
            op(base + 2);
        } else if id == ir.IN_FLOOR {
            push(a);
            op(base + 3);
        } else if id == ir.IN_FTRUNC {
            push(a);
            op(base + 4);
        } else if id == ir.IN_NEAREST {
            push(a);
            op(base + 5);
        } else if id == ir.IN_SQRT {
            push(a);
            op(base + 6);
        } else if id == ir.IN_FMIN {
            push(a);
            push(v.xs.get(1));
            op(base + 11);
        } else if id == ir.IN_FMAX {
            push(a);
            push(v.xs.get(1));
            op(base + 12);
        } else if id == ir.IN_COPYSIGN {
            push(a);
            push(v.xs.get(1));
            op(base + 13);
        } else {
            fail(u.f1("float intrinsic %s", u.idec(id)));
        }
        return;
    }
    // integers
    let bits = t.bits;
    let w = is64(t);
    if id == ir.IN_CTLZ {
        push_ext(a, false);
        iop(t, 0x67, 0x79);
        if bits < 32 {
            i32c(@as(i64, 32 - bits));
            op(0x6b);
        }
        return;
    }
    if id == ir.IN_CTTZ {
        push(a);
        if bits < 32 {
            i32c(@as(i64, 1) << @int_cast(u32, bits));
            op(0x72);
        }
        iop(t, 0x68, 0x7a);
        return;
    }
    if id == ir.IN_CTPOP {
        push_ext(a, false);
        iop(t, 0x69, 0x7b);
        return;
    }
    if (id == ir.IN_ROTL) or (id == ir.IN_ROTR) {
        let k = v.xs.get(1);
        if bits >= 32 {
            push(a);
            push(k);
            if id == ir.IN_ROTL {
                iop(t, 0x77, 0x89);
            } else {
                iop(t, 0x78, 0x8a);
            }
            return;
        }
        // narrow: (x << k | x >> (bits - k)) with k taken mod bits
        let x = new_local(wo.I32);
        let kk = new_local(wo.I32);
        push_ext(a, false);
        local_set(x);
        push(k);
        i32c(@as(i64, bits - 1));
        op(0x71);
        if id == ir.IN_ROTR {
            // rotr k = rotl (bits - k) mod bits
            local_set(kk);
            i32c(@as(i64, bits));
            local_get(kk);
            op(0x6b);
            i32c(@as(i64, bits - 1));
            op(0x71);
        }
        local_set(kk);
        local_get(x);
        local_get(kk);
        op(0x74);
        local_get(x);
        i32c(@as(i64, bits));
        local_get(kk);
        op(0x6b);
        op(0x76);
        op(0x72);
        return;
    }
    if (id == ir.IN_SMIN) or (id == ir.IN_SMAX) or (id == ir.IN_UMIN) or (id == ir.IN_UMAX) {
        let sg = (id == ir.IN_SMIN) or (id == ir.IN_SMAX);
        let x = new_local(vt(t));
        let y = new_local(vt(t));
        push_ext(a, sg);
        local_tee(x);
        push_ext(v.xs.get(1), sg);
        local_tee(y);
        local_get(x);
        local_get(y);
        if id == ir.IN_SMIN {
            icmp_op(ir.IntSLT, w);
        } else if id == ir.IN_SMAX {
            icmp_op(ir.IntSGT, w);
        } else if id == ir.IN_UMIN {
            icmp_op(ir.IntULT, w);
        } else {
            icmp_op(ir.IntUGT, w);
        }
        op(0x1b);
        return;
    }
    if id == ir.IN_ABS {
        let x = new_local(vt(t));
        push_ext(a, true);
        local_set(x);
        if w {
            i64c(0);
        } else {
            i32c(0);
        }
        local_get(x);
        iop(t, 0x6b, 0x7d);
        local_get(x);
        local_get(x);
        if w {
            i64c(0);
        } else {
            i32c(0);
        }
        icmp_op(ir.IntSLT, w);
        op(0x1b);
        return;
    }
    if (id == ir.IN_SADD_SAT) or (id == ir.IN_UADD_SAT) or (id == ir.IN_SSUB_SAT) or (id == ir.IN_USUB_SAT) {
        scalar_sat(v, id);
        return;
    }
    fail(u.f1("intrinsic %s", u.idec(id)));
}

// Saturating add / sub on scalars: computed wide (i64 for <= 32 bits) and
// clamped; 64-bit uses the overflow tests.
fn scalar_sat(v: *mut ir.Value, id: i32) void {
    let t = v.ty;
    let bits = t.bits;
    let sg = (id == ir.IN_SADD_SAT) or (id == ir.IN_SSUB_SAT);
    let is_add = (id == ir.IN_SADD_SAT) or (id == ir.IN_UADD_SAT);
    if bits <= 32 {
        let r = new_local(wo.I64);
        push_ext(v.xs.get(0), sg);
        op(u.pick(sg, "\xac", "\xad")[0]);
        push_ext(v.xs.get(1), sg);
        op(u.pick(sg, "\xac", "\xad")[0]);
        op(u.pick(is_add, "\x7c", "\x7d")[0]);
        local_set(r);
        let mut lo: i64 = 0;
        let mut hi: i64 = 0;
        if sg {
            lo = -(@as(i64, 1) << @int_cast(u32, bits - 1));
            hi = (@as(i64, 1) << @int_cast(u32, bits - 1)) - 1;
        } else {
            hi = @bit_cast(i64, (@as(u64, 1) << @int_cast(u32, bits)) - 1);
        }
        // clamp in i64: r < lo ? lo : (r > hi ? hi : r)
        i64c(lo);
        i64c(hi);
        local_get(r);
        local_get(r);
        i64c(hi);
        op(0x55);  // i64.gt_s
        op(0x1b);
        let r2 = new_local(wo.I64);
        local_tee(r2);
        local_get(r2);
        i64c(lo);
        op(0x53);  // i64.lt_s
        op(0x1b);
        op(0xa7);
        return;
    }
    // 64-bit
    let x = new_local(wo.I64);
    let y = new_local(wo.I64);
    let r = new_local(wo.I64);
    push(v.xs.get(0));
    local_set(x);
    push(v.xs.get(1));
    local_set(y);
    local_get(x);
    local_get(y);
    op(u.pick(is_add, "\x7c", "\x7d")[0]);
    local_set(r);
    if not sg {
        if is_add {
            // r < x: max
            i64c(-1);
            local_get(r);
            local_get(r);
            local_get(x);
            op(0x54);  // lt_u
            op(0x1b);
        } else {
            // x < y: 0
            i64c(0);
            local_get(r);
            local_get(x);
            local_get(y);
            op(0x54);
            op(0x1b);
        }
        return;
    }
    // signed: overflow if ((x ^ r) & (y ^ r)) < 0 (add), ((x ^ y) & (x ^ r)) < 0 (sub)
    // saturated value: x < 0 ? MIN : MAX
    local_get(x);
    i64c(63);
    op(0x87);  // shr_s: 0 or -1
    i64c(9223372036854775807);
    op(0x85);  // xor: MAX or MIN
    local_get(r);
    if is_add {
        local_get(x);
        local_get(r);
        op(0x85);
        local_get(y);
        local_get(r);
        op(0x85);
    } else {
        local_get(x);
        local_get(y);
        op(0x85);
        local_get(x);
        local_get(r);
        op(0x85);
    }
    op(0x83);
    i64c(0);
    op(0x53);
    op(0x1b);
}

fn gen_vec_intr(v: *mut ir.Value, id: i32) void {
    let a = v.xs.get(0);
    let at = a.ty;
    if id == ir.IN_ANYTRUE {
        push(a);
        op_fd(83);
        return;
    }
    if id == ir.IN_ALLTRUE {
        push(a);
        let codes: [6]u64 = [99, 131, 163, 195, 163, 195];
        op_fd(@as(u64, codes[@int_cast(usize, lane_shape(at))]));
        return;
    }
    if id == ir.IN_BITMASK {
        push(a);
        let codes: [6]u64 = [100, 132, 164, 196, 164, 196];
        op_fd(@as(u64, codes[@int_cast(usize, lane_shape(at))]));
        return;
    }
    if id == ir.IN_SWIZZLE {
        push(a);
        push(v.xs.get(1));
        op_fd(14);
        return;
    }
    if id == ir.IN_DOT_I16 {
        push(a);
        push(v.xs.get(1));
        op_fd(186);
        return;
    }
    let t = v.ty;
    let sh = lane_shape(t);
    if sh >= 4 {
        let f64_ = sh == 5;
        if id == ir.IN_FABS {
            push(a);
            op_fd(@as(u64, u.pick(f64_, "\xec", "\xe0")[0]));
        } else if id == ir.IN_SQRT {
            push(a);
            op_fd(@as(u64, u.pick(f64_, "\xef", "\xe3")[0]));
        } else if id == ir.IN_CEIL {
            push(a);
            op_fd(@as(u64, u.pick(f64_, "\x74", "\x67")[0]));
        } else if id == ir.IN_FLOOR {
            push(a);
            op_fd(@as(u64, u.pick(f64_, "\x75", "\x68")[0]));
        } else if id == ir.IN_FTRUNC {
            push(a);
            op_fd(@as(u64, u.pick(f64_, "\x7a", "\x69")[0]));
        } else if id == ir.IN_NEAREST {
            push(a);
            if f64_ {
                op_fd(148);
            } else {
                op_fd(106);
            }
        } else if (id == ir.IN_FMIN) or (id == ir.IN_FMAX) {
            push(a);
            push(v.xs.get(1));
            let mut c: u64 = 232;
            if f64_ {
                c = 244;
            }
            if id == ir.IN_FMAX {
                c += 1;
            }
            op_fd(c);
        } else if id == ir.IN_FMA {
            vec_fma(v);
        } else {
            fail(u.f1("vector float intrinsic %s", u.idec(id)));
        }
        return;
    }
    if id == ir.IN_ABS {
        push(a);
        let codes: [4]u64 = [96, 128, 160, 192];
        op_fd(@as(u64, codes[@int_cast(usize, sh)]));
        return;
    }
    if (id == ir.IN_SMIN) or (id == ir.IN_UMIN) or (id == ir.IN_SMAX) or (id == ir.IN_UMAX) {
        if sh == 3 {
            // i64x2: bitselect(a, b, a < b)
            let x = new_local(wo.V128);
            let y = new_local(wo.V128);
            push(a);
            local_tee(x);
            push(v.xs.get(1));
            local_tee(y);
            local_get(x);
            if (id == ir.IN_UMIN) or (id == ir.IN_UMAX) {
                flip_i64_signs();
            }
            local_get(y);
            if (id == ir.IN_UMIN) or (id == ir.IN_UMAX) {
                flip_i64_signs();
            }
            if (id == ir.IN_SMIN) or (id == ir.IN_UMIN) {
                op_fd(216);  // lt_s
            } else {
                op_fd(217);  // gt_s
            }
            op_fd(82);
            return;
        }
        push(a);
        push(v.xs.get(1));
        let base: [3]u64 = [118, 150, 182];
        let mut k: u64 = 0;
        if id == ir.IN_UMIN {
            k = 1;
        } else if id == ir.IN_SMAX {
            k = 2;
        } else if id == ir.IN_UMAX {
            k = 3;
        }
        op_fd(@as(u64, base[@int_cast(usize, sh)]) + k);
        return;
    }
    if (id == ir.IN_SADD_SAT) or (id == ir.IN_UADD_SAT) or (id == ir.IN_SSUB_SAT) or (id == ir.IN_USUB_SAT) {
        if sh <= 1 {
            push(a);
            push(v.xs.get(1));
            let mut base: u64 = 111;
            if sh == 1 {
                base = 143;
            }
            if id == ir.IN_UADD_SAT {
                base += 1;
            } else if id == ir.IN_SSUB_SAT {
                base += 3;
            } else if id == ir.IN_USUB_SAT {
                base += 4;
            }
            op_fd(base);
            return;
        }
        vec_lanewise(v);
        return;
    }
    if id == ir.IN_CTPOP {
        if sh == 0 {
            push(a);
            op_fd(98);
            return;
        }
    }
    vec_lanewise(v);
}

// Per-lane fallback: extract each lane, apply the scalar operation, rebuild.
fn vec_lanewise(v: *mut ir.Value) void {
    let t = v.ty;
    let n = @as(u64, @bit_cast(u32, t.lanes));
    let lt = t.elem;
    let xs = u.newn.[u32](v.xs.len + 1);
    for i in 0..v.xs.len {
        xs[i] = new_local(wo.V128);
        push(v.xs.get(i));
        local_set(xs[i]);
    }
    let r = new_local(wo.V128);
    op_fd(12);
    C.zeros(16);
    local_set(r);
    // the scalar intrinsic over lane values held in locals (as pseudo
    // parameters: a parameter's index is its local)
    for lane in @as(u64, 0)..n {
        local_get(r);
        let ls = u.newn.[*mut ir.Value](v.xs.len + 1);
        for i in 0..v.xs.len {
            let tl = new_local(vt(lt));
            local_get(xs[i]);
            extract_lane(t, lane, false);
            local_set(tl);
            let pv = ir.new_param(F, lt, @as(usize, tl));
            ls[i] = pv;
        }
        let fake = ir.new_inst(F, ir.OP_INTR, lt);
        fake.cv = v.cv;
        for i in 0..v.xs.len {
            fake.xs.push(ls[i]);
        }
        fake.flag = FL_INLINE;
        gen_intr(fake);
        replace_lane(t, lane);
        local_set(r);
    }
    local_get(r);
}

fn vec_fma(v: *mut ir.Value) void {
    vec_lanewise(v);
}

fn gen_shuffle(v: *mut ir.Value) void {
    push(v.a);
    push(v.b);
    let lb = @int_cast(usize, v.a.ty.elem.size);
    op_fd(13);
    for i in 0..v.mask.len {
        let m = v.mask[i];
        for k in 0..lb {
            let mut idx = @int_cast(usize, m) * lb + k;
            if m < 0 {
                idx = 0;
            }
            C.u8_(@truncate(u8, idx));
        }
    }
}

fn frame_addr(p: *mut ir.Value) void {
    local_get(fp_local);
    if p.aux != 0 {
        pconst(@bit_cast(u64, p.aux));
        padd_sub(false);
    }
}

// The code computing v's value (or effect) onto the stack.
fn expr(v: *mut ir.Value) void {
    let o = v.op;
    if o == ir.OP_ALLOCA {
        frame_addr(v);
        return;
    }
    if (o >= ir.OP_ADD) and (o <= ir.OP_FREM) {
        gen_binop(v);
        return;
    }
    if o == ir.OP_FNEG {
        push(v.a);
        if v.ty.kind == ir.T_VEC {
            op_fd(@as(u64, u.pick(lane_shape(v.ty) == 4, "\xe1", "\xed")[0]));
        } else {
            op(u.pick(v.ty.bits == 32, "\x8c", "\x9a")[0]);
        }
        return;
    }
    if o == ir.OP_ICMP {
        gen_icmp(v);
        return;
    }
    if o == ir.OP_FCMP {
        gen_fcmp(v);
        return;
    }
    if o == ir.OP_SELECT {
        push(v.b);
        push(v.c);
        push(v.a);
        op(0x1b);
        return;
    }
    if (o >= ir.OP_ZEXT) and (o <= ir.OP_FPTOUI_SAT) {
        gen_cast(v);
        return;
    }
    if o == ir.OP_LOAD {
        gen_load(v);
        return;
    }
    if o == ir.OP_STORE {
        gen_store(v);
        return;
    }
    if o == ir.OP_PTRADD {
        push(v.a);
        push(v.b);
        padd_sub(false);
        return;
    }
    if (o == ir.OP_MEMCPY) or (o == ir.OP_MEMMOVE) or (o == ir.OP_MEMSET) {
        gen_memop(v);
        return;
    }
    if o == ir.OP_RMW {
        gen_rmw(v);
        return;
    }
    if o == ir.OP_CMPXCHG {
        gen_cmpxchg(v);
        return;
    }
    if o == ir.OP_FENCE {
        if tg.has(tg.W_ATOMICS) {
            op_fe(3);
            op(0);
        }
        return;
    }
    if o == ir.OP_EXTRACTELEM {
        push(v.a);
        extract_lane(v.a.ty, v.cv, false);
        return;
    }
    if o == ir.OP_INSERTELEM {
        push(v.a);
        push(v.b);
        replace_lane(v.ty, v.cv);
        return;
    }
    if o == ir.OP_SPLAT {
        push(v.a);
        splat_op(v.ty);
        return;
    }
    if o == ir.OP_SHUFFLE {
        gen_shuffle(v);
        return;
    }
    if o == ir.OP_CALL {
        gen_call(v);
        return;
    }
    if o == ir.OP_INTR {
        gen_intr(v);
        return;
    }
    fail(u.f1("cannot generate %s", ir.inst_str(v)));
}

// -----------------------------------------------------------------------------
// Value classification
// -----------------------------------------------------------------------------
fn pure_op(v: *ir.Value) bool {
    let o = v.op;
    if (o >= ir.OP_ADD) and (o <= ir.OP_FPTOUI_SAT) {
        return true;
    }
    if (o == ir.OP_PTRADD) or (o == ir.OP_EXTRACTELEM) or (o == ir.OP_INSERTELEM) or (o == ir.OP_SPLAT) or
       (o == ir.OP_SHUFFLE) {
        return true;
    }
    if o == ir.OP_INTR {
        let id = @truncate(i32, @bit_cast(i64, v.cv));
        return not ((id == ir.IN_TRAP) or (id == ir.IN_MEMGROW) or (id == ir.IN_MEMSIZE) or (id == ir.IN_WAIT32) or
                    (id == ir.IN_NOTIFY) or (id == ir.IN_SET_SP));
    }
    return false;
}

fn count_uses(f: *mut ir.Func) void {
    let mut b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        let mut pos: i32 = 0;
        while u.notnil(i) {
            i.uses = 0;
            i.user = vnil();
            i.pos = pos;
            i.loc = -1;
            i.flag = FL_LOCAL;
            pos += 1;
            i = i.next;
        }
        b = b.next;
    }
    b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            let ops: [3]*mut ir.Value = [i.a, i.b, i.c];
            for o in ops {
                if u.notnil(o) and (o.kind == ir.V_INST) {
                    o.uses += 1;
                    o.user = i;
                }
            }
            for k in 0..i.xs.len {
                let o = i.xs.get(k);
                if o.kind == ir.V_INST {
                    o.uses += 1;
                    o.user = i;
                }
            }
            i = i.next;
        }
        b = b.next;
    }
}

// The block phi p takes v from (nil: none).
fn phi_from(p: *ir.Value, v: *mut ir.Value) *mut ir.Block {
    for k in 0..p.xs.len {
        if p.xs.get(k) == v {
            return p.bs.get(k);
        }
    }
    return u.nil.[ir.Block]();
}

fn classify(f: *mut ir.Func) void {
    let mut b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            let o = i.op;
            if o == ir.OP_ALLOCA {
                i.flag = FL_REMAT;
            } else if (o == ir.OP_PTRADD) and (i.b.kind == ir.V_CINT) and (i.uses > 1) and
                      ((i.a.kind != ir.V_INST) or (i.a.flag == FL_LOCAL) or (i.a.flag == FL_REMAT)) {
                i.flag = FL_REMAT;
            } else if (i.uses == 1) and (o != ir.OP_PHI) and pure_op(i) and (i.user.op == ir.OP_PHI) and
                      phi_from(i.user, i) == b {
                // computed where the phi copy at the end of this block pushes it
                i.flag = FL_INLINE;
            } else if (i.uses == 1) and (o != ir.OP_PHI) and not ir.is_term(o) and (i.user.blk == b) and
                      (i.user.op != ir.OP_PHI) and (i.user.pos > i.pos) {
                if pure_op(i) {
                    i.flag = FL_INLINE;
                } else if (o == ir.OP_LOAD) or (o == ir.OP_CALL) or (o == ir.OP_INTR) or (o == ir.OP_RMW) or
                          (o == ir.OP_CMPXCHG) {
                    // only if nothing with effects happens in between
                    let mut ok = true;
                    let mut x = i.next;
                    while x != i.user {
                        if not pure_op(x) and (x.op != ir.OP_ALLOCA) {
                            ok = false;
                            break;
                        }
                        x = x.next;
                    }
                    if ok {
                        i.flag = FL_INLINE;
                    }
                }
            }
            i = i.next;
        }
        b = b.next;
    }
}

// -----------------------------------------------------------------------------
// Control flow
// -----------------------------------------------------------------------------
fn depth_of(kind: i32, target: *mut ir.Block) u64 {
    let n = ctx.len;
    for k in 0..n {
        let c = ctx.get(n - 1 - k);
        if (c.kind == kind) and (c.target == target) {
            return @as(u64, k);
        }
    }
    fail(u.f1("no enclosing label for block %s", target.name));
    return 0;
}

fn phi_copies(from: *mut ir.Block, to: *mut ir.Block) void {
    let mut n: usize = 0;
    let mut p = to.first;
    while u.notnil(p) and (p.op == ir.OP_PHI) {
        let mut found = false;
        for k in 0..p.bs.len {
            if p.bs.get(k) == from {
                push(p.xs.get(k));
                found = true;
                break;
            }
        }
        if not found {
            push_const(ir.undef(p.ty));
        }
        n += 1;
        p = p.next;
    }
    // set in reverse order
    let mut last = to.first;
    for k in 0..n {
        last = last.next;
    }
    let mut q = to.first;
    let ps = u.newn.[*mut ir.Value](n + 1);
    for k in 0..n {
        ps[k] = q;
        q = q.next;
    }
    let mut k = n;
    while k > 0 {
        k -= 1;
        local_set(@int_cast(u32, ps[k].loc));
    }
}

// Pushes the negation of i1 value c: an inlined integer comparison with the
// inverse predicate, else c and i32.eqz.
fn push_not(c: *mut ir.Value) void {
    if (c.kind == ir.V_INST) and (c.op == ir.OP_ICMP) and (c.flag == FL_INLINE) and (c.a.ty.kind != ir.T_VEC) {
        let p = cb.inv_pred(@truncate(i32, @bit_cast(i64, c.cv)));
        let sg = is_signed_pred(p);
        push_ext(c.a, sg);
        push_ext(c.b, sg);
        icmp_op(p, is64(c.a.ty));
        return;
    }
    push(c);
    op(0x45);  // i32.eqz
}

// Would a branch from `from` to `to` be just a `br` (no phi copies)?
fn plain_branch(from: *mut ir.Block, to: *mut ir.Block) bool {
    if u.notnil(to.first) and (to.first.op == ir.OP_PHI) {
        return false;
    }
    return (to.rpo <= from.rpo) or is_merge(to);
}

fn branch_depth(from: *mut ir.Block, to: *mut ir.Block) u64 {
    if to.rpo <= from.rpo {
        return depth_of(CTX_LOOP, to);
    }
    return depth_of(CTX_BLOCK, to);
}

fn do_branch(from: *mut ir.Block, to: *mut ir.Block) void {
    phi_copies(from, to);
    if to.rpo <= from.rpo {
        op(0x0c);
        C.uleb(depth_of(CTX_LOOP, to));
        return;
    }
    if is_merge(to) {
        op(0x0c);
        C.uleb(depth_of(CTX_BLOCK, to));
        return;
    }
    do_tree(to);
}

fn do_tree(x: *mut ir.Block) void {
    // merge children, in decreasing reverse postorder
    let mut ys = u.List.[*mut ir.Block].{ data: u.nil.[*mut ir.Block](), len: 0, cap: 0 };
    for k in 0..x.dom_kids.len {
        let y = x.dom_kids.get(k);
        if is_merge(y) {
            ys.push(y);
        }
    }
    // sort by rpo descending (small lists: insertion sort)
    for i in 1..ys.len {
        let mut j = i;
        while (j > 0) and (ys.get(j - 1).rpo < ys.get(j).rpo) {
            let t = ys.get(j - 1);
            ys.set(j - 1, ys.get(j));
            ys.set(j, t);
            j -= 1;
        }
    }
    if is_header(x) {
        op(0x03);
        op(0x40);
        ctx.push(Ctx.{ kind: CTX_LOOP, target: x });
        node_within(x, ys.items());
        _ = ctx.pop();
        op(0x0b);
    } else {
        node_within(x, ys.items());
    }
    u.free(@ptr_cast(*mut void, ys.data));
}

fn node_within(x: *mut ir.Block, ys: []mut *mut ir.Block) void {
    if ys.len > 0 {
        let y = ys[0];
        op(0x02);
        op(0x40);
        ctx.push(Ctx.{ kind: CTX_BLOCK, target: y });
        node_within(x, ys[1..]);
        _ = ctx.pop();
        op(0x0b);
        do_tree(y);
        return;
    }
    emit_block(x);
}

fn emit_block(x: *mut ir.Block) void {
    let mut i = x.first;
    while u.notnil(i) {
        let o = i.op;
        if (o == ir.OP_PHI) or ir.is_term(o) or (i.flag != FL_LOCAL) {
            i = i.next;
            continue;
        }
        expr(i);
        if i.ty.kind != ir.T_VOID {
            if i.uses > 0 {
                local_set(@int_cast(u32, i.loc));
            } else {
                op(0x1a);
            }
        }
        i = i.next;
    }
    let t = ir.terminator(x);
    let o = t.op;
    if o == ir.OP_BR {
        do_branch(x, t.t1);
    } else if (o == ir.OP_CONDBR) and plain_branch(x, t.t1) {
        // br_if to the target, then the other side inline
        push(t.a);
        op(0x0d);
        C.uleb(branch_depth(x, t.t1));
        do_branch(x, t.t2);
    } else if (o == ir.OP_CONDBR) and plain_branch(x, t.t2) {
        push_not(t.a);
        op(0x0d);
        C.uleb(branch_depth(x, t.t2));
        do_branch(x, t.t1);
    } else if o == ir.OP_CONDBR {
        push(t.a);
        op(0x04);
        op(0x40);
        ctx.push(Ctx.{ kind: CTX_IF, target: u.nil.[ir.Block]() });
        do_branch(x, t.t1);
        op(0x05);
        do_branch(x, t.t2);
        _ = ctx.pop();
        op(0x0b);
    } else if o == ir.OP_SWITCH {
        emit_switch(x, t);
    } else if o == ir.OP_RET {
        if u.notnil(t.a) {
            arg_ext(t.a, F.ret_ext);
        }
        epilogue();
        op(0x0f);
    } else {
        op(0x00);
    }
}

fn emit_switch(x: *mut ir.Block, t: *mut ir.Value) void {
    let n = t.xs.len;
    // dense range of case values?
    let mut lo: i64 = 0;
    let mut hi: i64 = 0;
    for k in 0..n {
        let c = ir.sval(t.xs.get(k));
        if (k == 0) or (c < lo) {
            lo = c;
        }
        if (k == 0) or (c > hi) {
            hi = c;
        }
    }
    let span = hi - lo + 1;
    if (n == 0) or (span > (@as(i64, @int_cast(i32, n)) * 3 + 8)) or (span > 10000) {
        // compare chain
        let v = new_local(wo.I64);
        let sg = true;
        push_ext(t.a, sg);
        if not is64(t.a.ty) {
            op(0xac);
        }
        local_set(v);
        for k in 0..n {
            local_get(v);
            i64c(ir.sval(t.xs.get(k)));
            op(0x51);
            op(0x04);
            op(0x40);
            ctx.push(Ctx.{ kind: CTX_IF, target: u.nil.[ir.Block]() });
            do_branch(x, t.bs.get(k));
            _ = ctx.pop();
            op(0x0b);
        }
        do_branch(x, t.t1);
        return;
    }
    // targets: 0..n-1 cases, n default; one nested block per case edge
    let nt = n + 1;
    for k in 0..nt {
        op(0x02);
        op(0x40);
        ctx.push(Ctx.{ kind: CTX_OTHER, target: u.nil.[ir.Block]() });
    }
    // index = value - lo (as i32), with out-of-range going to default
    let idx = new_local(wo.I32);
    push_ext(t.a, true);
    if is64(t.a.ty) {
        i64c(lo);
        op(0x7d);
        let w = new_local(wo.I64);
        local_tee(w);
        i64c(span);
        op(0x5a);  // ge_u
        op(0x04);
        op(0x40);
        i32c(@as(i64, @int_cast(i32, n)));
        local_set(idx);
        op(0x05);
        local_get(w);
        op(0xa7);
        local_set(idx);
        op(0x0b);
    } else {
        i32c(lo);
        op(0x6b);
        local_set(idx);
    }
    // br_table: depth k for case index; table entries for each value in range
    local_get(idx);
    op(0x0e);
    C.uleb(@bit_cast(u64, span));
    for j in 0..@int_cast(usize, span) {
        let val = lo + @as(i64, @int_cast(i32, j));
        let mut target = n;  // default
        for k in 0..n {
            if ir.sval(t.xs.get(k)) == val {
                target = k;
                break;
            }
        }
        C.uleb(@as(u64, target));
    }
    C.uleb(@as(u64, n));
    for k in 0..nt {
        _ = ctx.pop();
        op(0x0b);
        if k < n {
            do_branch(x, t.bs.get(k));
        } else {
            do_branch(x, t.t1);
        }
    }
}

fn epilogue() void {
    if frame_size > 0 {
        local_get(fp_local);
        pconst(frame_size);
        padd_sub(false);
        op(0x24);
        reloc(wo.R_GLOBAL_INDEX_LEB, sp_sym, 0);
        C.uleb5(0);
    }
}

// -----------------------------------------------------------------------------
// Functions
// -----------------------------------------------------------------------------
fn gen_function(f: *mut ir.Func, wf: *mut wo.Func) void {
    F = f;
    C = bn.new_bytes();
    set_end = 0xffff_ffff;
    RL = u.List.[wo.Reloc].{ data: u.nil.[wo.Reloc](), len: 0, cap: 0 };
    LT = u.List.[u8].{ data: u.nil.[u8](), len: 0, cap: 0 };
    ctx.clear();
    nparams = @int_cast(u32, f.params.len);
    opt.compute_preds(f);
    let order = opt.dominators(f);
    let nb = order.len;
    let merge = u.newn.[bool](nb + 1);
    let header = u.newn.[bool](nb + 1);
    merge_p = merge.ptr;
    header_p = header.ptr;
    nblocks = nb;
    for i in 0..nb {
        let b = order.get(i);
        let mut fwd: usize = 0;
        for k in 0..b.preds.len {
            let p = b.preds.get(k);
            if p.rpo < 0 {
                continue;
            }
            if p.rpo < b.rpo {
                fwd += 1;
            } else {
                header[i] = true;
            }
        }
        merge[i] = fwd >= 2;
    }
    count_uses(f);
    classify(f);
    // frame: allocas
    frame_size = 0;
    let mut b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            if i.op == ir.OP_ALLOCA {
                let al = @as(u64, i.align);
                let mut a = i.aty.align;
                if al > a {
                    a = al;
                }
                if a > 16 {
                    a = 16;
                }
                if a == 0 {
                    a = 1;
                }
                frame_size = (frame_size + a - 1) & ~(a - 1);
                i.aux = @bit_cast(i64, frame_size);
                frame_size += i.aty.size;
            }
            i = i.next;
        }
        b = b.next;
    }
    // one buffer for the variadic arguments of any call
    let mut va: u64 = 0;
    b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            if (i.op == ir.OP_CALL) and i.fty.vararg {
                let n = va_buffer_size(i);
                if n > va {
                    va = n;
                }
            }
            i = i.next;
        }
        b = b.next;
    }
    if va > 0 {
        frame_size = (frame_size + 15) & ~@as(u64, 15);
        va_off = @bit_cast(i64, frame_size);
        frame_size += va;
    }
    frame_size = (frame_size + 15) & ~@as(u64, 15);
    if frame_size > 0 {
        fp_local = new_local(ptr_vt());
    }
    // locals for values
    b = f.first;
    while u.notnil(b) {
        let mut i = b.first;
        while u.notnil(i) {
            if (i.ty.kind == ir.T_VEC) and not tg.has(tg.W_SIMD128) {
                fail("vector types need the simd128 extension (--features=+simd128)");
                return;
            }
            if (i.ty.kind != ir.T_VOID) and (i.flag == FL_LOCAL) and ((i.uses > 0) or (i.op == ir.OP_PHI)) {
                i.loc = @int_cast(i32, new_local(vt(i.ty)));
            }
            i = i.next;
        }
        b = b.next;
    }
    // prologue
    if frame_size > 0 {
        op(0x23);
        reloc(wo.R_GLOBAL_INDEX_LEB, sp_sym, 0);
        C.uleb5(0);
        pconst(frame_size);
        padd_sub(true);
        local_tee(fp_local);
        op(0x24);
        reloc(wo.R_GLOBAL_INDEX_LEB, sp_sym, 0);
        C.uleb5(0);
    }
    do_tree(order.get(0));
    if f.ty.ret.kind != ir.T_VOID {
        op(0x00);
    }
    op(0x0b);
    // body = locals (run-length encoded) + code
    let mut body = bn.new_bytes();
    let mut groups: u64 = 0;
    let mut k: usize = 0;
    while k < LT.len {
        let mut j = k;
        while (j < LT.len) and (LT.get(j) == LT.get(k)) {
            j += 1;
        }
        groups += 1;
        k = j;
    }
    body.uleb(groups);
    k = 0;
    while k < LT.len {
        let mut j = k;
        while (j < LT.len) and (LT.get(j) == LT.get(k)) {
            j += 1;
        }
        body.uleb(@as(u64, j - k));
        body.u8_(LT.get(k));
        k = j;
    }
    let base = @int_cast(u32, body.len());
    body.put(C.view());
    wf.body = body;
    for i in 0..RL.len {
        let mut r = RL.get(i);
        r.off += base;
        wf.relocs.push(r);
    }
    C.free();
    u.free(@ptr_cast(*mut void, RL.data));
    u.free(@ptr_cast(*mut void, LT.data));
    u.free(@ptr_cast(*mut void, order.data));
}

// -----------------------------------------------------------------------------
// Module
// -----------------------------------------------------------------------------
fn sym_flags_for(linkage: i32, defined: bool) u32 {
    let mut fl: u32 = 0;
    if not defined {
        fl = wo.F_UNDEFINED;
        if linkage == ir.LINK_EXTWEAK {
            fl = fl | wo.F_WEAK;
        }
        return fl;
    }
    if linkage == ir.LINK_INTERNAL {
        fl = wo.F_LOCAL;
    } else if (linkage == ir.LINK_LINKONCE) or (linkage == ir.LINK_WEAK) {
        fl = wo.F_WEAK | wo.F_HIDDEN;
    } else {
        fl = wo.F_HIDDEN;
    }
    return fl;
}

fn emit_data(g: *mut ir.Global, seg: *mut wo.Seg, x: *mut ir.Value, off: u64) void {
    let t = x.ty;
    let k = x.kind;
    if k == ir.V_GLOBAL {
        let s = gsyms.get(@int_cast(usize, x.gv.id));
        if W64 {
            seg.relocs.push(wo.Reloc.{ ty: wo.R_MEMORY_ADDR_I64, off: @truncate(u32, off), sym: s, addend: 0 });
        } else {
            seg.relocs.push(wo.Reloc.{ ty: wo.R_MEMORY_ADDR_I32, off: @truncate(u32, off), sym: s, addend: 0 });
        }
        return;
    }
    if k == ir.V_FUNC {
        UN.uses_table = true;
        let s = fsyms.get(@int_cast(usize, x.fnp.id));
        if W64 {
            seg.relocs.push(wo.Reloc.{ ty: wo.R_TABLE_INDEX_I64, off: @truncate(u32, off), sym: s, addend: 0 });
        } else {
            seg.relocs.push(wo.Reloc.{ ty: wo.R_TABLE_INDEX_I32, off: @truncate(u32, off), sym: s, addend: 0 });
        }
        return;
    }
    if (k == ir.V_CAGG) and (t.kind != ir.T_VEC) {
        for i in 0..x.xs.len {
            emit_data(g, seg, x.xs.get(i), off + ir.field_off(t, i));
        }
        return;
    }
    const_bytes(x, &mut seg.data, @truncate(usize, off));
}

pub fn compile(m: *mut ir.Module, wasm64: bool) *mut wo.Unit {
    W64 = wasm64;
    errors = false;
    UN = wo.new_unit(wasm64);
    fsyms.clear();
    gsyms.clear();
    // __stack_pointer (always global import 0)
    UN.gimports.push(wo.Import.{ module: "env", name: "__stack_pointer", ty: @as(u32, ptr_vt()), mutable: true });
    sp_sym = wo.add_sym(UN, wo.Sym.{ kind: wo.SYM_GLOBAL, flags: wo.F_UNDEFINED, name: "__stack_pointer", index: 0, seg: 0,
                                     off: 0, size: 0 });
    // globals: wasm globals (imports first), then data
    for i in 0..m.globals.len {
        gsyms.push(0);
    }
    for i in 0..m.globals.len {
        let g = m.globals.get(i);
        if g.wasm_global and u.isnil(g.init) {
            let gi = @int_cast(u32, UN.gimports.len);
            UN.gimports.push(wo.Import.{ module: "env", name: g.name, ty: @as(u32, vt(g.ty)), mutable: true });
            gsyms.set(i, wo.add_sym(UN, wo.Sym.{ kind: wo.SYM_GLOBAL, flags: wo.F_UNDEFINED, name: g.name, index: gi,
                                                 seg: 0, off: 0, size: 0 }));
        }
    }
    for i in 0..m.globals.len {
        let g = m.globals.get(i);
        if g.wasm_global and u.notnil(g.init) {
            let gi = @int_cast(u32, UN.gimports.len + UN.globals.len);
            UN.globals.push(wo.Global.{ vt: vt(g.ty), mutable: true, init: 0 });
            gsyms.set(i, wo.add_sym(UN, wo.Sym.{ kind: wo.SYM_GLOBAL, flags: sym_flags_for(g.linkage, true),
                                                 name: g.name, index: gi, seg: 0, off: 0, size: 0 }));
        } else if not g.wasm_global {
            if u.isnil(g.init) {
                gsyms.set(i, wo.add_sym(UN, wo.Sym.{ kind: wo.SYM_DATA, flags: sym_flags_for(g.linkage, false),
                                                     name: g.name, index: 0, seg: 0, off: 0, size: 0 }));
            } else {
                let seg = u.new.[wo.Seg]();
                let mut al: u32 = 0;
                while (@as(u64, 1) << al) < g.align {
                    al += 1;
                }
                seg.align = al;
                let pre = u.pick(g.is_const, ".rodata.", u.pick(ir.is_zero(g.init), ".bss.", ".data."));
                seg.name = u.cat2(pre, g.name);
                seg.tls = false;
                UN.segs.push(seg);
                gsyms.set(i, wo.add_sym(UN, wo.Sym.{ kind: wo.SYM_DATA, flags: sym_flags_for(g.linkage, true),
                                                     name: g.name, index: 0, seg: @int_cast(u32, UN.segs.len - 1),
                                                     off: 0, size: g.ty.size }));
            }
        }
    }
    // functions: imports, then definitions
    for i in 0..m.funcs.len {
        fsyms.push(0);
    }
    for i in 0..m.funcs.len {
        let f = m.funcs.get(i);
        if not ir.is_decl(f) {
            continue;
        }
        let fi = @int_cast(u32, UN.fimports.len);
        let mut modname: []u8 = "env";
        let mut nm = f.name;
        let mut fl = sym_flags_for(f.linkage, false);
        if f.import_module.len > 0 {
            modname = f.import_module;
            nm = f.import_name;
            fl = fl | wo.F_EXPLICIT_NAME;
        }
        UN.fimports.push(wo.Import.{ module: modname, name: nm, ty: fn_type(f.ty), mutable: false });
        fsyms.set(i, wo.add_sym(UN, wo.Sym.{ kind: wo.SYM_FUNC, flags: fl, name: f.name, index: fi, seg: 0, off: 0,
                                             size: 0 }));
    }
    for i in 0..m.funcs.len {
        let f = m.funcs.get(i);
        if ir.is_decl(f) {
            continue;
        }
        let fi = @int_cast(u32, UN.fimports.len + UN.funcs.len);
        let wf = u.new.[wo.Func]();
        wf.ty = fn_type(f.ty);
        wf.name = f.name;
        let mut fl = sym_flags_for(f.linkage, true);
        if f.export_name.len > 0 {
            fl = (fl | wo.F_EXPORTED | wo.F_NO_STRIP) & ~wo.F_HIDDEN;
            UN.exports.push(wo.Export.{ name: f.export_name, func: fi });
        }
        wf.sym = wo.add_sym(UN, wo.Sym.{ kind: wo.SYM_FUNC, flags: fl, name: f.name, index: fi, seg: 0, off: 0,
                                         size: 0 });
        fsyms.set(i, wf.sym);
        UN.funcs.push(wf);
    }
    // data
    for i in 0..m.globals.len {
        let g = m.globals.get(i);
        if g.wasm_global or u.isnil(g.init) {
            continue;
        }
        let s = UN.syms.get(@as(usize, gsyms.get(i)));
        let seg = UN.segs.get(@as(usize, s.seg));
        seg.data.zeros(@truncate(usize, g.ty.size));
        emit_data(g, seg, g.init, 0);
    }
    // code
    let mut k: usize = 0;
    for i in 0..m.funcs.len {
        let f = m.funcs.get(i);
        if ir.is_decl(f) {
            continue;
        }
        gen_function(f, UN.funcs.get(k));
        k += 1;
    }
    if errors {
        return u.nil.[wo.Unit]();
    }
    return UN;
}

// codegen.c - lowering of the checked AST to LLVM IR via the LLVM C API.
//
// Generic functions are monomorphized here: each (decl, type arguments)
// pair becomes one LLVM function, generated from the shared, already
// type-checked body by substituting the type arguments.
#include "codegen.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>

#include <llvm-c/Analysis.h>
#include <llvm-c/Core.h>
#include <llvm-c/DebugInfo.h>
#include <llvm-c/Support.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/Transforms/PassBuilder.h>

#include "builtins.h"
#include "diag.h"
#include "target.h"

// sema helpers reused by codegen
bool is_place(Expr *e);

SymVec g_export_names;
SymVec g_link_libs;
bool g_test_mode;
bool g_threads;

static CgOptions *O;
static LLVMContextRef ctx;
static LLVMModuleRef mod;
static LLVMBuilderRef B;
static LLVMTargetMachineRef TM;
static LLVMTargetDataRef TD;
static LLVMTypeRef T_i1, T_i8, T_i16, T_i32, T_i64, T_i128, T_isz, T_ptr, T_void, T_f32, T_f64, T_v128;

enum { IDX_ARRAY, IDX_SLICE, IDX_PTR_ARRAY };

// ---------------------------------------------------------------------------
// Function instances
// ---------------------------------------------------------------------------
typedef struct FnInst {
    Decl *decl;
    Type **gargs;
    int ngargs;
    Type *fnty;  // concrete TY_FN
    LLVMValueRef fn;
    LLVMTypeRef llty;
    bool sret;
    const char *display;  // user-facing name, for stack traces and profiles
    struct FnInst *next;
    uint64_t feats;  // the processor extensions its code may use
    bool dispatch;   // @features / @multiversion: chooses a version and calls it
    Sym key;
    int ninl;           // inline fn: trailing parameters for its callers' call sites (INL_SITES or 0)
    LLVMTypeRef ref_llty;  // its type without them
    LLVMValueRef ref;   // inline fn used as a value: a function without them that calls it
} FnInst;

static SymMap inst_map;
static FnInst *pending_head, *pending_tail;

typedef struct LoopCtx {
    LLVMBasicBlockRef brk, cont;
    int defer_height;
} LoopCtx;

typedef struct FnState {
    FnInst *inst;
    Decl *decl;
    Type **from, **to;
    int nsub;
    LLVMValueRef fn;
    LLVMValueRef *locals;
    Expr **inits;  // the initializers of immutable locals (lane masks: is_mask)
    int nlocals;
    LLVMValueRef sret_ptr;
    Type *ret;  // concrete
    Vec(Stmt *) defers;
    Vec(LoopCtx) loops;
    LLVMBasicBlockRef entry, trap_bb;
    LLVMValueRef *inl;  // inline fn: its callers' call-site records, innermost first (NULL: none)
    int ninl;
    LLVMMetadataRef di_sp;  // -g: the function's DISubprogram
    int di_file;            // and its source file
} FnState;

static FnState *F;

// ---------------------------------------------------------------------------
// Line tables (-g, --debug-info=lines): a compile unit per object, a
// DISubprogram per function and a DILocation per instruction, from the
// spans that the trap sites use. LLVM keeps them through inlining
// (DW_TAG_inlined_subroutine).
// ---------------------------------------------------------------------------
static LLVMDIBuilderRef DIB;
static LLVMMetadataRef di_cu;
static SymMap di_files;

static LLVMMetadataRef di_file(int id) {
    SourceFile *f = g_files.data[id];
    Sym key = intern_cstr(arena_printf("%d", id));
    LLVMMetadataRef m = symmap_get(&di_files, key);
    if (m) return m;
    char buf[PATH_MAX];
    const char *full = realpath(f->path, buf) ? buf : f->display;
    const char *slash = strrchr(full, '/');
    const char *dir = slash ? arena_strndup(full, (size_t)(slash - full)) : ".";
    const char *base = slash ? slash + 1 : full;
    m = LLVMDIBuilderCreateFile(DIB, base, strlen(base), dir, strlen(dir));
    symmap_put(&di_files, key, m);
    return m;
}

static void dbg_init(void) {
    DIB = NULL;
    di_files = (SymMap){0};
    if (!O->debug_info) return;
    DIB = LLVMCreateDIBuilder(mod);
    LLVMMetadataRef file = di_file(O->unit == CG_MODULE ? O->module->file->id : g_prog.root->file->id);
    const char *producer = "axiom";
    di_cu = LLVMDIBuilderCreateCompileUnit(DIB, LLVMDWARFSourceLanguageC, file, producer, strlen(producer),
                                           O->opt_level > 0, "", 0, 0, "", 0, LLVMDWARFEmissionLineTablesOnly, 0,
                                           false, false, "", 0, "", 0);
    LLVMAddModuleFlag(mod, LLVMModuleFlagBehaviorWarning, "Debug Info Version", 18,
                      LLVMValueAsMetadata(LLVMConstInt(T_i32, LLVMDebugMetadataVersion(), false)));
    LLVMAddModuleFlag(mod, LLVMModuleFlagBehaviorWarning, "Dwarf Version", 13,
                      LLVMValueAsMetadata(LLVMConstInt(T_i32, 4, false)));
}

// The current function's instructions are attributed to `sp` from here on.
static void dbg_at(Span sp) {
    if (!F || !F->di_sp || sp.file < 0 || sp.file >= g_files.len) return;
    int l, c, el, ec;
    span_linecol(sp, &l, &c, &el, &ec);
    LLVMMetadataRef scope = F->di_sp;
    if (sp.file != F->di_file) scope = LLVMDIBuilderCreateLexicalBlockFile(DIB, scope, di_file(sp.file), 0);
    LLVMSetCurrentDebugLocation2(B, LLVMDIBuilderCreateDebugLocation(ctx, (unsigned)l, (unsigned)c, scope, NULL));
}

// Code generated into another function (a helper made on first use) has no
// location from the current one.
static LLVMMetadataRef dbg_suspend(void) {
    LLVMMetadataRef loc = LLVMGetCurrentDebugLocation2(B);
    LLVMSetCurrentDebugLocation2(B, NULL);
    return loc;
}
static void dbg_resume(LLVMMetadataRef loc) { LLVMSetCurrentDebugLocation2(B, loc); }

static LLVMValueRef errcode_value(uint32_t code);
static bool type_mentions_error(Type *t);
static const char *export_symbol(Decl *d);
static bool g_sanitize_main;  // --sanitize=address: main is wrapped (emit_asan_main)
static LLVMValueRef error_name_fn(LLVMTypeRef *fty_out);
static LLVMValueRef shared_global(LLVMTypeRef t, const char *name, LLVMValueRef init, unsigned addrspace);

static LLVMValueRef gen(Expr *e);
static LLVMValueRef gen_inner(Expr *e);
static LLVMValueRef gen_addr_inner(Expr *e);
static LLVMValueRef gen_addr(Expr *e);
static void gen_stmt(Stmt *s);
static void gen_block(Stmt *b);
static LLVMTypeRef lty(Type *t);
static FnInst *get_inst(Decl *d, Type **gargs, int n);
static LLVMValueRef fn_ref(FnInst *fi);
static LLVMValueRef cpu_has_bits(uint64_t bits);
static void add_ext_attrs(LLVMValueRef v, Type *fnty, bool sret, bool is_call);

// The concrete type of t in the function being generated. An enum is its
// representation from here on: code generation never needs to tell them apart.
static Type *ct(Type *t) {
    if (t && t->has_params && F) t = type_subst(t, F->from, F->to, F->nsub);
    return type_lower_enums(t);
}

static void internal_error(const char *msg) {
    fprintf(stderr, "axiom: internal compiler error: %s\n", msg);
    exit(2);
}

// ---------------------------------------------------------------------------
// Types
// ---------------------------------------------------------------------------
static LLVMTypeRef align_unit(int64_t a) {
    switch (a) {
    case 1: return T_i8;
    case 2: return T_i16;
    case 4: return T_i32;
    case 8: return T_i64;
    default: return a >= 32 ? LLVMVectorType(T_i32, (unsigned)(a / 4)) : T_v128;
    }
}

static LLVMTypeRef lty(Type *t) {
    if (t->has_params) internal_error(arena_printf("unsubstituted generic type %s", type_str(t)));
    if (t->backend) return t->backend;
    if (t->has_enum) return t->backend = lty(type_lower_enums(t));
    LLVMTypeRef r = NULL;
    switch (t->kind) {
    case TY_VOID: r = T_void; break;
    case TY_BOOL: r = T_i8; break;
    case TY_INT: r = LLVMIntTypeInContext(ctx, (unsigned)t->bits); break;
    case TY_FLOAT: r = t->bits == 32 ? T_f32 : T_f64; break;
    case TY_VEC: {
        LLVMTypeRef lane = t->elem->kind == TY_FLOAT ? (t->elem->bits == 32 ? T_f32 : T_f64)
                                                     : LLVMIntTypeInContext(ctx, (unsigned)t->elem->bits);
        r = LLVMVectorType(lane, (unsigned)t->lanes);
        break;
    }
    case TY_V128: r = LLVMVectorType(T_i32, (unsigned)(t->bits / 32)); break;
    case TY_PTR:
    case TY_FN: r = T_ptr; break;
    case TY_ERROR: r = T_i16; break;
    case TY_SLICE: {
        LLVMTypeRef el[2] = {T_ptr, T_isz};
        r = LLVMStructTypeInContext(ctx, el, 2, false);
        break;
    }
    case TY_ARRAY: r = LLVMArrayType2(lty(t->elem), (uint64_t)t->len); break;
    case TY_ERRUNION: {
        if (t->elem->kind == TY_VOID) {
            LLVMTypeRef el[1] = {T_i16};
            r = LLVMStructTypeInContext(ctx, el, 1, false);
        } else {
            LLVMTypeRef el[2] = {T_i16, lty(t->elem)};
            r = LLVMStructTypeInContext(ctx, el, 2, false);
        }
        break;
    }
    case TY_STRUCT: {
        r = LLVMStructCreateNamed(ctx, type_str(t));
        t->backend = r;
        int n = t->decl->fields.len;
        LLVMTypeRef *el = NEWN(LLVMTypeRef, n + 1);
        for (int i = 0; i < n; i++) el[i] = lty(type_field(t, i));
        LLVMStructSetBody(r, el, (unsigned)n, false);
        break;
    }
    case TY_UNION: {
        r = LLVMStructCreateNamed(ctx, type_str(t));
        t->backend = r;
        LLVMTypeRef tag = lty(union_tag_type(t));
        int64_t psize = union_payload_size(t), palign = union_payload_align(t);
        if (psize == 0) {
            LLVMTypeRef el[1] = {tag};
            LLVMStructSetBody(r, el, 1, false);
        } else {
            LLVMTypeRef el[2] = {tag, LLVMArrayType2(align_unit(palign), (uint64_t)((psize + palign - 1) / palign))};
            LLVMStructSetBody(r, el, 2, false);
        }
        break;
    }
    default: internal_error(arena_printf("no LLVM type for %s", type_str(t)));
    }
    t->backend = r;
    return r;
}

// SSA representation: bool is i1; everything else as in memory.
static LLVMTypeRef vty(Type *t) { return t->kind == TY_BOOL ? T_i1 : lty(t); }

// Passed by pointer to a copy the callee only reads (results: through a
// pointer to the caller's memory): aggregates larger than 32 bytes, and
// 256- and 512-bit vectors, so that functions compiled for different
// processor extensions (spec §13.4) agree on how they arrive.
static bool by_ptr(Type *t) {
    if (ty_is_simd(t)) return type_size(t) > 16;
    return ty_is_aggregate(t) && type_size(t) > 32;
}

static LLVMTypeRef fn_lty(Type *fnty, bool *sret_out) {
    Type *ret = fnty->ret;
    bool sret = by_ptr(ret);
    int n = fnty->nparams + (sret ? 1 : 0);
    LLVMTypeRef *ps = NEWN(LLVMTypeRef, n + 1);
    int k = 0;
    if (sret) ps[k++] = T_ptr;
    for (int i = 0; i < fnty->nparams; i++) ps[k++] = by_ptr(fnty->params[i]) ? T_ptr : vty(fnty->params[i]);
    LLVMTypeRef rt = sret || ret->kind == TY_VOID ? T_void : vty(ret);
    if (sret_out) *sret_out = sret;
    return LLVMFunctionType(rt, ps, (unsigned)n, false);
}

// ---------------------------------------------------------------------------
// Small IR helpers
// ---------------------------------------------------------------------------
static LLVMValueRef cint(LLVMTypeRef t, i128 v) {
    if (LLVMGetIntTypeWidth(t) <= 64) return LLVMConstInt(t, (unsigned long long)(int64_t)v, true);
    uint64_t words[2] = {(uint64_t)v, (uint64_t)((unsigned __int128)v >> 64)};
    return LLVMConstIntOfArbitraryPrecision(t, 2, words);
}

static LLVMValueRef usz(int64_t v) { return LLVMConstInt(T_isz, (unsigned long long)v, false); }

static bool terminated(void) {
    LLVMBasicBlockRef bb = LLVMGetInsertBlock(B);
    return LLVMGetBasicBlockTerminator(bb) != NULL;
}

static LLVMBasicBlockRef new_bb(const char *name) { return LLVMAppendBasicBlockInContext(ctx, F->fn, name); }

// After a diverging instruction: continue in a fresh (unreachable) block.
static void dead_block(void) { LLVMPositionBuilderAtEnd(B, new_bb("dead")); }

// Set by setup_target: the native CPU can do @swizzle with pshufb.
static bool g_has_ssse3;

static LLVMValueRef intrinsic(const char *name, LLVMTypeRef *tys, int ntys, LLVMValueRef *args, int nargs) {
    unsigned id = LLVMLookupIntrinsicID(name, strlen(name));
    if (!id) internal_error(arena_printf("unknown intrinsic %s", name));
    LLVMValueRef fn = LLVMGetIntrinsicDeclaration(mod, id, tys, (size_t)ntys);
    LLVMTypeRef fty = LLVMIntrinsicGetType(ctx, id, tys, (size_t)ntys);
    return LLVMBuildCall2(B, fty, fn, args, (unsigned)nargs, "");
}

static LLVMValueRef intr1(const char *name, LLVMValueRef a) {
    LLVMTypeRef t = LLVMTypeOf(a);
    return intrinsic(name, &t, 1, &a, 1);
}
static LLVMValueRef intr2(const char *name, LLVMValueRef a, LLVMValueRef b) {
    LLVMTypeRef t = LLVMTypeOf(a);
    LLVMValueRef args[2] = {a, b};
    return intrinsic(name, &t, 1, args, 2);
}

// ---------------------------------------------------------------------------
// Traps (spec Appendix B).
//
// Every trap site has a constant record {code, file id, span, line, column,
// file name, source text}. Before trapping, the helper stores the record's
// address (and, for @expect_eq, the compared values) in axiom.trap_info,
// which hosts read after the trap (wasm export axiom_trap_info; native
// weak hook ax_on_trap). Going through one cold helper also keeps LLVM's fast
// instruction selector usable at -O0; -O1+ inlines it.
// ---------------------------------------------------------------------------
enum {
    T_TRAP = 0, T_BOUNDS = 1, T_OVERFLOW = 2, T_DIVZERO = 3, T_INTCAST = 4, T_LENGTH = 6, T_UNIONTAG = 7,
    T_STACK = 8, T_ATOMIC_ALIGN = 10, T_EXPECT = 11, T_MEMORY = 12, T_CPU = 13,
    SITE_ERR_RETURN = 254, SITE_CALL = 255,  // not traps: shadow stack records
};

static LLVMValueRef string_ptr(const char *bytes, uint32_t len);
static LLVMBasicBlockRef new_bb(const char *name);
static Span g_cur_span;
static LLVMValueRef trap_fn, trap_info, trap_hook;
static LLVMTypeRef trap_fnty, site_ty;
static SymMap site_map;

// axiom.trap_info: {site, kind, a, b, alen, blen}, all 64-bit.
static LLVMValueRef trap_info_global(void) {
    if (!trap_info) {
        LLVMTypeRef t = LLVMArrayType2(T_i64, 6);
        trap_info = shared_global(t, "axiom.trap_info", LLVMConstNull(t), 0);
        LLVMSetAlignment(trap_info, 8);
    }
    return trap_info;
}

static LLVMValueRef trap_info_slot(int i) {
    LLVMValueRef ix[2] = {LLVMConstInt(T_i32, 0, false), LLVMConstInt(T_i32, (unsigned)i, false)};
    return LLVMBuildGEP2(B, LLVMArrayType2(T_i64, 6), trap_info_global(), ix, 2, "");
}

uint32_t file_key(int file_id) {
    if (file_id < 0 || file_id >= g_files.len) return 0xFFFFFFFFu;
    uint32_t h = 2166136261u;
    for (const char *p = g_files.data[file_id]->display; *p; p++) h = (h ^ (uint8_t)*p) * 16777619u;
    return h == 0xFFFFFFFFu ? 0 : h;
}

void collect_link_names(const CgOptions *o) {
    g_sanitize_main = o->sanitize_address;
    SymMap seen = {0};
    for (int i = 0; i < g_prog.fns.len; i++) {
        Decl *d = g_prog.fns.data[i];
        if (!d->is_export) continue;
        Sym s = intern_cstr(export_symbol(d));
        if (!symmap_get(&seen, s)) vec_push(g_export_names, s);
        symmap_put(&seen, s, (void *)1);
    }
    for (int i = 0; i < g_prog.host_fns.len; i++) {
        Decl *d = g_prog.host_fns.data[i];
        if (!d->extern_c || !d->link_lib) continue;
        Sym s = intern_cstr(d->link_lib);
        if (!symmap_get(&seen, s)) vec_push(g_link_libs, s);
        symmap_put(&seen, s, (void *)1);
    }
}

static const char *cur_fn_display(void) { return F && F->inst ? F->inst->display : ""; }

// A constant site record {code, file id, start, end, line, col, file, text,
// fn, callee}. `fn` is the enclosing function; `callee` is set for call sites.
// Codes: a trap code, SITE_CALL for a call, SITE_ERR_RETURN for an error return.
static LLVMValueRef site_record_ex(int code, Span sp, const char *callee) {
    const char *fn = cur_fn_display();
    if (!callee) callee = "";
    Sym key = intern_cstr(arena_printf("%d:%d:%u:%u:%s:%s", code, sp.file, sp.start, sp.end, fn, callee));
    LLVMValueRef g = symmap_get(&site_map, key);
    if (g) return g;
    if (!site_ty) {
        LLVMTypeRef el[10] = {T_i32, T_i32, T_i32, T_i32, T_i32, T_i32, T_ptr, T_ptr, T_ptr, T_ptr};
        site_ty = LLVMStructTypeInContext(ctx, el, 10, false);
    }
    int sl = 0, sc = 0, el_ = 0, ec = 0;
    const char *file = "";
    const char *text = "";
    if (sp.file >= 0 && sp.file < g_files.len && sp.end >= sp.start) {
        span_linecol(sp, &sl, &sc, &el_, &ec);
        SourceFile *f = g_files.data[sp.file];
        file = f->display;
        uint32_t n = sp.end - sp.start;
        uint32_t max = code >= SITE_ERR_RETURN ? 80 : 200;
        if (n > max) n = max;
        // one line of text only
        for (uint32_t i = 0; i < n; i++)
            if (f->text[sp.start + i] == '\n') {
                n = i;
                break;
            }
        text = arena_strndup(f->text + sp.start, n);
    }
    LLVMValueRef vals[10] = {
        LLVMConstInt(T_i32, (unsigned)code, false), LLVMConstInt(T_i32, file_key(sp.file), false),
        LLVMConstInt(T_i32, sp.start, false),       LLVMConstInt(T_i32, sp.end, false),
        LLVMConstInt(T_i32, (unsigned)sl, false),   LLVMConstInt(T_i32, (unsigned)sc, false),
        string_ptr(file, (uint32_t)strlen(file)),   string_ptr(text, (uint32_t)strlen(text)),
        string_ptr(fn, (uint32_t)strlen(fn)),       string_ptr(callee, (uint32_t)strlen(callee)),
    };
    g = LLVMAddGlobal(mod, site_ty, ".site");
    LLVMSetInitializer(g, LLVMConstStructInContext(ctx, vals, 10, false));
    LLVMSetLinkage(g, LLVMPrivateLinkage);
    LLVMSetGlobalConstant(g, true);
    LLVMSetAlignment(g, 8);
    symmap_put(&site_map, key, g);
    return g;
}

static LLVMValueRef site_record(int code, Span sp) { return site_record_ex(code, sp, NULL); }

// ---------------------------------------------------------------------------
// Shadow stack (spec §16.4): each thread has
//     {u32 depth, u32 errors, [SS_CAP]*Site calls, [ET_CAP]*Site error_returns}
// Every call stores its call-site record at calls[depth % SS_CAP] and bumps
// depth for the duration of the call, so after a trap or crash the host can
// print the innermost SS_CAP frames with exact source locations. Error
// returns are appended to a ring the same way; `catch` clears it. On wasm the
// record's address lives in a per-instance (so per-thread) wasm global; on
// native targets the record is thread-local.
// ---------------------------------------------------------------------------
#define SS_CAP 256
#define ET_CAP 32
static LLVMTypeRef ss_ty;
static LLVMValueRef ss_global, ss_main;

// WebAssembly stacks live in linear memory, where running past the end goes
// unnoticed: each function with a frame there checks it against this
// per-thread limit (a wasm global) first. The main thread's stack is the
// first MiB of memory (--stack-first); its lowest KiB stays unused.
#define WASM_MAIN_STACK_LIMIT 1024
static LLVMValueRef stack_limit_global;

static LLVMValueRef stack_limit(void) {
    if (!stack_limit_global)
        stack_limit_global = shared_global(T_isz, "axiom.stack_limit", usz(WASM_MAIN_STACK_LIMIT), 1);
    return stack_limit_global;
}

static LLVMValueRef stack_pointer_global(void) {
    LLVMValueRef sp = LLVMGetNamedGlobal(mod, "__stack_pointer");
    if (!sp) {
        sp = LLVMAddGlobalInAddressSpace(mod, T_ptr, "__stack_pointer", 1);
        LLVMSetLinkage(sp, LLVMExternalLinkage);
    }
    return sp;
}

static bool stack_traces_on(void) { return O && O->stack_traces; }

static LLVMTypeRef ss_type(void) {
    if (!ss_ty) {
        LLVMTypeRef el[4] = {T_i32, T_i32, LLVMArrayType2(T_ptr, SS_CAP), LLVMArrayType2(T_ptr, ET_CAP)};
        ss_ty = LLVMStructTypeInContext(ctx, el, 4, false);
    }
    return ss_ty;
}

static void ss_globals(void) {
    LLVMTypeRef t = ss_type();
    if (!ss_global) {
        if (g_target.wasm) {
            ss_main = shared_global(t, "axiom.ss.main", LLVMConstNull(t), 0);
            LLVMSetAlignment(ss_main, 8);
            // null means the main thread's record (wasm-ld does not relocate
            // wasm global initializers); thread starts set their own.
            ss_global = shared_global(T_ptr, "axiom.ss", LLVMConstNull(T_ptr), 1);
        } else {
            ss_global = shared_global(t, "axiom.ss", LLVMConstNull(t), 0);
            LLVMSetThreadLocal(ss_global, true);
            LLVMSetThreadLocalMode(ss_global, LLVMInitialExecTLSModel);
            LLVMSetAlignment(ss_global, 8);
        }
    }
}

static LLVMValueRef ss_ptr(void) {
    ss_globals();
    if (g_target.wasm) {
        LLVMValueRef p = LLVMBuildLoad2(B, T_ptr, ss_global, "");
        return LLVMBuildSelect(B, LLVMBuildIsNull(B, p, ""), ss_main, p, "ss");
    }
    return ss_global;
}

static LLVMValueRef ss_field(LLVMValueRef ss, unsigned i) { return LLVMBuildStructGEP2(B, ss_type(), ss, i, ""); }

static LLVMValueRef ss_slot(LLVMValueRef ss, unsigned field, LLVMValueRef idx) {
    LLVMValueRef ix[3] = {LLVMConstInt(T_i32, 0, false), LLVMConstInt(T_i32, field, false), idx};
    return LLVMBuildGEP2(B, ss_type(), ss, ix, 3, "");
}

typedef struct SsFrame {
    LLVMValueRef ss, depth;
} SsFrame;

// In profiling builds another thread (or a signal handler) reads the stack at
// any moment, so the stores must happen even where nothing in this thread
// reads them (after inlining, an increment followed by the restore is
// otherwise a dead store).
static void ss_store(LLVMValueRef v, LLVMValueRef p) {
    LLVMValueRef st = LLVMBuildStore(B, v, p);
    if (O->profile) LLVMSetVolatile(st, true);
}

// Inline functions (spec §9.5) have no call-site records of their own: a
// call to one passes the record as an argument instead (with those of the
// inline calls that led to the caller, innermost first, up to INL_SITES),
// and the function pushes them only where they are needed: before it calls
// a function that is not inline, and before it traps. Once inlined the
// arguments are constants, so a call to an inline function costs nothing
// at run time. (Profiling builds record every call, so that time spent in
// inline functions is attributed to them.)
#define INL_SITES 4

static bool inline_sites_on(void) { return stack_traces_on() && !O->profile; }

// Pushes call-site record `p` unless it is null (a constant once inlined).
static void ss_push_site(LLVMValueRef ss, LLVMValueRef p) {
    LLVMBasicBlockRef doit = new_bb("ss.inline"), next = new_bb("ss.inline.done");
    LLVMBuildCondBr(B, LLVMBuildIsNotNull(B, p, ""), doit, next);
    LLVMPositionBuilderAtEnd(B, doit);
    LLVMValueRef dp = ss_field(ss, 0);
    LLVMValueRef d = LLVMBuildLoad2(B, T_i32, dp, "");
    ss_store(p, ss_slot(ss, 2, LLVMBuildAnd(B, d, LLVMConstInt(T_i32, SS_CAP - 1, false), "")));
    ss_store(LLVMBuildAdd(B, d, LLVMConstInt(T_i32, 1, false), ""), dp);
    LLVMBuildBr(B, next);
    LLVMPositionBuilderAtEnd(B, next);
}

// In an inline function: pushes its callers' records [lo, hi), outermost first.
static void ss_push_inline_sites(LLVMValueRef ss, int lo, int hi) {
    for (int i = hi - 1; i >= lo; i--) ss_push_site(ss, F->inl[i]);
}

static SsFrame ss_push(const char *callee) {
    SsFrame f = {0};
    if (!stack_traces_on() || !F) return f;
    f.ss = ss_ptr();
    LLVMValueRef dp = ss_field(f.ss, 0);
    f.depth = LLVMBuildLoad2(B, T_i32, dp, "depth");
    LLVMValueRef d = f.depth;
    if (F->ninl) {
        ss_push_inline_sites(f.ss, 0, F->ninl);
        d = LLVMBuildLoad2(B, T_i32, dp, "depth");
    }
    LLVMValueRef idx = LLVMBuildAnd(B, d, LLVMConstInt(T_i32, SS_CAP - 1, false), "");
    ss_store(site_record_ex(SITE_CALL, g_cur_span, callee), ss_slot(f.ss, 2, idx));
    ss_store(LLVMBuildAdd(B, d, LLVMConstInt(T_i32, 1, false), ""), dp);
    return f;
}

static void ss_pop(SsFrame f) {
    if (f.ss) ss_store(f.depth, ss_field(f.ss, 0));
}

// Appends an error-return record when `code` (an error code, i16) is nonzero.
static void etrace_push(LLVMValueRef code) {
    if (!stack_traces_on() || !F) return;
    if (LLVMIsAConstantInt(code) && LLVMConstIntGetZExtValue(code) == 0) return;
    LLVMBasicBlockRef cont = NULL;
    if (!LLVMIsAConstantInt(code)) {
        LLVMBasicBlockRef rec = new_bb("etrace");
        cont = new_bb("etrace.done");
        LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntNE, code, LLVMConstNull(LLVMTypeOf(code)), ""), rec, cont);
        LLVMPositionBuilderAtEnd(B, rec);
    }
    LLVMValueRef ss = ss_ptr();
    LLVMValueRef np = ss_field(ss, 1);
    LLVMValueRef n = LLVMBuildLoad2(B, T_i32, np, "");
    LLVMValueRef idx = LLVMBuildAnd(B, n, LLVMConstInt(T_i32, ET_CAP - 1, false), "");
    LLVMBuildStore(B, site_record(SITE_ERR_RETURN, g_cur_span), ss_slot(ss, 3, idx));
    LLVMBuildStore(B, LLVMBuildAdd(B, n, LLVMConstInt(T_i32, 1, false), ""), np);
    if (cont) {
        LLVMBuildBr(B, cont);
        LLVMPositionBuilderAtEnd(B, cont);
    }
}

static void etrace_clear(void) {
    if (!stack_traces_on() || !F) return;
    LLVMBuildStore(B, LLVMConstInt(T_i32, 0, false), ss_field(ss_ptr(), 1));
}

static void call_trap_site(LLVMValueRef site) {
    if (F && F->ninl) ss_push_inline_sites(ss_ptr(), 0, F->ninl);  // (a trap does not return)
    if (!trap_fn) {
        trap_fnty = LLVMFunctionType(T_void, &T_ptr, 1, false);
        trap_fn = LLVMAddFunction(mod, "axiom.trap", trap_fnty);
        LLVMSetLinkage(trap_fn, LLVMInternalLinkage);
        const char *attrs[] = {"noreturn", "cold", "nounwind", "noinline"};
        for (int i = 0; i < 4; i++) {
            unsigned k = LLVMGetEnumAttributeKindForName(attrs[i], strlen(attrs[i]));
            LLVMAddAttributeAtIndex(trap_fn, LLVMAttributeFunctionIndex, LLVMCreateEnumAttribute(ctx, k, 0));
        }
        LLVMBasicBlockRef cur = LLVMGetInsertBlock(B);
        LLVMMetadataRef loc = dbg_suspend();
        LLVMValueRef fn = trap_fn;
        LLVMPositionBuilderAtEnd(B, LLVMAppendBasicBlockInContext(ctx, fn, "entry"));
        LLVMValueRef sp = LLVMGetParam(fn, 0);
        LLVMBuildStore(B, LLVMBuildZExt(B, LLVMBuildPtrToInt(B, sp, T_isz, ""), T_i64, ""), trap_info_slot(0));
        if (!g_target.wasm) {
            // Optional host hook (runtime/native_host.c, runtime/test_main.c).
            LLVMTypeRef ps[3] = {T_ptr, T_ptr, T_ptr};
            LLVMTypeRef hty = LLVMFunctionType(T_void, ps, 3, false);
            trap_hook = LLVMAddFunction(mod, "ax_on_trap", hty);
            LLVMSetLinkage(trap_hook, LLVMExternalWeakLinkage);
            LLVMBasicBlockRef call_bb = LLVMAppendBasicBlockInContext(ctx, fn, "hook");
            LLVMBasicBlockRef done_bb = LLVMAppendBasicBlockInContext(ctx, fn, "trap");
            LLVMValueRef has = LLVMBuildICmp(B, LLVMIntNE, trap_hook, LLVMConstNull(T_ptr), "");
            LLVMBuildCondBr(B, has, call_bb, done_bb);
            LLVMPositionBuilderAtEnd(B, call_bb);
            LLVMValueRef args[3] = {sp, trap_info_global(), stack_traces_on() ? ss_ptr() : LLVMConstNull(T_ptr)};
            LLVMBuildCall2(B, hty, trap_hook, args, 3, "");
            LLVMBuildBr(B, done_bb);
            LLVMPositionBuilderAtEnd(B, done_bb);
        }
        intrinsic("llvm.trap", NULL, 0, NULL, 0);
        LLVMBuildUnreachable(B);
        LLVMPositionBuilderAtEnd(B, cur);
        dbg_resume(loc);
    }
    LLVMBuildCall2(B, trap_fnty, trap_fn, &site, 1, "");
}

// A block that traps with `code` at the current source span.
static LLVMBasicBlockRef trap_block(int code) {
    LLVMBasicBlockRef cur = LLVMGetInsertBlock(B);
    LLVMBasicBlockRef bb = new_bb("trap");
    LLVMPositionBuilderAtEnd(B, bb);
    call_trap_site(site_record(code, g_cur_span));
    LLVMBuildUnreachable(B);
    LLVMPositionBuilderAtEnd(B, cur);
    return bb;
}

// Continue only if `ok` holds; otherwise trap with `code`.
static void check_or_trap(LLVMValueRef ok, int code) {
    if (LLVMIsAConstantInt(ok) && LLVMConstIntGetZExtValue(ok) == 1) return;
    LLVMBasicBlockRef cont = new_bb("ok");
    LLVMBuildCondBr(B, ok, cont, trap_block(code));
    LLVMPositionBuilderAtEnd(B, cont);
}

static void emit_trap(int code) {
    call_trap_site(site_record(code, g_cur_span));
    LLVMBuildUnreachable(B);
    dead_block();
}

static LLVMValueRef alloca_in_entry(LLVMTypeRef t, const char *name) {
    LLVMBasicBlockRef cur = LLVMGetInsertBlock(B);
    LLVMMetadataRef loc = LLVMGetCurrentDebugLocation2(B);
    LLVMValueRef term = LLVMGetBasicBlockTerminator(F->entry);
    if (term) LLVMPositionBuilderBefore(B, term);
    else LLVMPositionBuilderAtEnd(B, F->entry);
    LLVMValueRef a = LLVMBuildAlloca(B, t, name);
    LLVMPositionBuilderAtEnd(B, cur);
    dbg_resume(loc);
    return a;
}

static LLVMValueRef temp_alloca(Type *t) {
    LLVMValueRef a = alloca_in_entry(lty(t), "tmp");
    LLVMSetAlignment(a, (unsigned)type_align(t));
    return a;
}

static LLVMValueRef to_mem(Type *t, LLVMValueRef v) {
    if (t->kind == TY_BOOL) return LLVMBuildZExt(B, v, T_i8, "");
    return v;
}
static LLVMValueRef from_mem(Type *t, LLVMValueRef v) {
    if (t->kind == TY_BOOL) return LLVMBuildICmp(B, LLVMIntNE, v, LLVMConstInt(T_i8, 0, false), "");
    return v;
}

// align: 0 = natural (known-aligned storage), 1 = unknown (through pointers)
static LLVMValueRef load(Type *t, LLVMValueRef p, bool unaligned) {
    LLVMValueRef v = LLVMBuildLoad2(B, lty(t), p, "");
    if (unaligned) LLVMSetAlignment(v, 1);
    return from_mem(t, v);
}
// Aggregates are first-class values here, but LLVM's instruction selection
// takes a large one apart element by element (and fails on very large ones):
// zero it with memset, and copy a constant one (from a global) or one just
// loaded from memory with memmove.
#define BIG_AGGREGATE 256

static void store(Type *t, LLVMValueRef v, LLVMValueRef p, bool unaligned) {
    if (ty_is_aggregate(t) && type_size(t) > BIG_AGGREGATE) {
        LLVMValueRef size = usz(type_size(t));
        if (LLVMIsConstant(v) && LLVMIsNull(v)) {
            LLVMBuildMemSet(B, p, LLVMConstInt(T_i8, 0, false), size, 1);
            return;
        }
        if (LLVMIsConstant(v)) {
            LLVMValueRef g = LLVMAddGlobal(mod, LLVMTypeOf(v), "aggregate.const");
            LLVMSetInitializer(g, v);
            LLVMSetGlobalConstant(g, true);
            LLVMSetLinkage(g, LLVMPrivateLinkage);
            LLVMSetUnnamedAddress(g, LLVMGlobalUnnamedAddr);
            LLVMBuildMemMove(B, p, 1, g, 1, size);
            return;
        }
        if (LLVMIsALoadInst(v) && !LLVMGetFirstUse(v)) {
            LLVMValueRef src = LLVMGetOperand(v, 0);
            LLVMInstructionEraseFromParent(v);
            LLVMBuildMemMove(B, p, 1, src, 1, size);
            return;
        }
    }
    LLVMValueRef s = LLVMBuildStore(B, to_mem(t, v), p);
    if (unaligned) LLVMSetAlignment(s, 1);
}

static LLVMValueRef spill(Type *t, LLVMValueRef v) {
    LLVMValueRef a = temp_alloca(t);
    store(t, v, a, false);
    return a;
}

static LLVMValueRef local_ptr(Local *l) {
    if (!F->locals[l->index]) {
        Type *t = ct(l->type);
        LLVMValueRef a = alloca_in_entry(lty(t), l->name);
        LLVMSetAlignment(a, (unsigned)type_align(t));
        F->locals[l->index] = a;
    }
    return F->locals[l->index];
}

// ---------------------------------------------------------------------------
// Strings and globals
// ---------------------------------------------------------------------------
static SymMap string_globals;

static LLVMValueRef string_ptr(const char *bytes, uint32_t len) {
    Sym key = intern(bytes, len);
    LLVMValueRef g = symmap_get(&string_globals, key);
    if (!g) {
        // A NUL follows every literal (not part of the slice), so that
        // "text".ptr is a valid C string (spec §2.5).
        LLVMValueRef init = LLVMConstStringInContext(ctx, bytes, len, false);
        g = LLVMAddGlobal(mod, LLVMArrayType2(T_i8, (uint64_t)len + 1), ".str");
        LLVMSetInitializer(g, init);
        LLVMSetLinkage(g, LLVMPrivateLinkage);
        LLVMSetGlobalConstant(g, true);
        LLVMSetUnnamedAddress(g, LLVMGlobalUnnamedAddr);
        LLVMSetAlignment(g, 1);
        symmap_put(&string_globals, key, g);
    }
    return g;
}

static LLVMValueRef const_slice(LLVMValueRef ptr, int64_t len) {
    LLVMValueRef el[2] = {ptr, usz(len)};
    return LLVMConstStructInContext(ctx, el, 2, false);
}

static SymMap global_vals;  // Decl* name -> LLVMValueRef (keyed by arena string of pointer)

static Sym ptr_key(void *p) { return intern_cstr(arena_printf("%p", p)); }

static LLVMValueRef global_ref(Decl *d);

// Constant for a global initializer (spec: constant expressions only).
static LLVMValueRef gen_const(Expr *e, Type *t) {
    t = ct(t);
    if (e->kind == E_CONV) {
        switch ((ConvKind)e->op) {
        case CONV_NOOP: return gen_const(e->a, t);
        case CONV_WRAP_OK: {
            if (t->elem->kind == TY_VOID) {
                LLVMValueRef el[1] = {LLVMConstInt(T_i16, 0, false)};
                return LLVMConstStructInContext(ctx, el, 1, false);
            }
            LLVMValueRef el[2] = {LLVMConstInt(T_i16, 0, false), gen_const(e->a, t->elem)};
            return LLVMConstStructInContext(ctx, el, 2, false);
        }
        case CONV_WRAP_ERR: {
            LLVMValueRef code = gen_const(e->a, t_error);
            if (t->elem->kind == TY_VOID) {
                LLVMValueRef el[1] = {code};
                return LLVMConstStructInContext(ctx, el, 1, false);
            }
            LLVMValueRef el[2] = {code, LLVMConstNull(lty(t->elem))};
            return LLVMConstStructInContext(ctx, el, 2, false);
        }
        }
    }
    if (e->is_const && (t->kind == TY_INT)) return cint(lty(t), e->cval);
    if (e->is_const && t->kind == TY_FLOAT) return LLVMConstReal(lty(t), e->fval);
    if (t->kind == TY_BOOL && (e->kind == E_BOOL || e->is_const)) return LLVMConstInt(T_i8, e->bval ? 1 : 0, false);
    switch (e->kind) {
    case E_FLOAT: return LLVMConstReal(lty(t), e->fval);
    case E_STRING: return const_slice(string_ptr(e->str, e->slen), e->slen);
    case E_ERROR_VAL: return LLVMConstInt(T_i16, (unsigned long long)e->cval, false);
    case E_ARRAY_LIT: {
        int n = e->args.len;
        LLVMValueRef *vs = NEWN(LLVMValueRef, n + 1);
        for (int i = 0; i < n; i++) vs[i] = gen_const(e->args.data[i], t->elem);
        return LLVMConstArray2(lty(t->elem), vs, (uint64_t)n);
    }
    case E_ARRAY_REPEAT: {
        LLVMValueRef v = gen_const(e->a, t->elem);
        if (LLVMIsNull(v)) return LLVMConstNull(lty(t));
        LLVMValueRef *vs = NEWN(LLVMValueRef, t->len + 1);
        for (int64_t i = 0; i < t->len; i++) vs[i] = v;
        return LLVMConstArray2(lty(t->elem), vs, (uint64_t)t->len);
    }
    case E_STRUCT_LIT: {
        int n = t->decl->fields.len;
        LLVMValueRef *vs = NEWN(LLVMValueRef, n + 1);
        for (int i = 0; i < n; i++) vs[i] = gen_const(e->inits.data[i].value, type_field(t, i));
        return LLVMConstNamedStruct(lty(t), vs, (unsigned)n);
    }
    case E_FIELD:
        if (e->op == FK_VARIANT_UNIT) {
            LLVMTypeRef ut = lty(t);
            LLVMValueRef tag = LLVMConstInt(lty(union_tag_type(t)), (unsigned long long)e->index, false);
            if (LLVMCountStructElementTypes(ut) == 1) return LLVMConstNamedStruct(ut, &tag, 1);
            LLVMValueRef el[2] = {tag, LLVMConstNull(LLVMStructGetTypeAtIndex(ut, 1))};
            return LLVMConstNamedStruct(ut, el, 2);
        }
        if (e->op == FK_DECL && e->decl && e->decl->kind == D_FN) return fn_ref(get_inst(e->decl, NULL, 0));
        break;
    case E_IDENT:
        if (e->decl && e->decl->kind == D_FN) return fn_ref(get_inst(e->decl, NULL, 0));
        if (e->decl && e->decl->kind == D_HOST_FN) return get_inst(e->decl, NULL, 0)->fn;
        break;
    case E_BUILTIN:
        if (e->index == BI_PTR_FROM_INT) return LLVMConstIntToPtr(gen_const(e->bargs.data[1].e, t_usize), T_ptr);
        break;
    case E_UNARY:
        if (e->op == TK_MINUS && t->kind == TY_FLOAT) return LLVMConstReal(lty(t), -e->a->fval);
        break;
    case E_CALL:
        if (e->op == CALLK_VARIANT) {
            diag_error(e->span, "E0353", "union constants with a payload are not supported by this compiler yet");
            return LLVMConstNull(lty(t));
        }
        break;
    default: break;
    }
    diag_error(e->span, "E0350", "global initializer must be a constant expression");
    return LLVMConstNull(lty(t));
}

// Symbols are prefixed with the module's path (without .ax), which is unique
// within a build, so modules compiled separately never clash.
static const char *mangle_prefix(Decl *d) {
    const char *base = d->module ? g_files.data[d->module->file->id]->display : "";
    size_t n = strlen(base);
    if (n > 3 && strcmp(base + n - 3, ".ax") == 0) n -= 3;
    return arena_strndup(base, n);
}

// A type in a symbol name: like type_str, but struct and union names are
// qualified by their module, so instances are named the same in every unit.
static void mangle_type_into(CharVec *b, Type *t) {
#define MPUT(str)                                                  \
    do {                                                           \
        for (const char *q_ = (str); *q_; q_++) vec_push(*b, *q_); \
    } while (0)
    switch (t->kind) {
    case TY_STRUCT:
    case TY_UNION:
        if (t->decl->anon) {  // anonymous structs are the same type everywhere
            MPUT("struct{");
            for (int i = 0; i < t->decl->fields.len; i++) {
                if (i) MPUT(",");
                MPUT(t->decl->fields.data[i].name);
                MPUT(":");
                mangle_type_into(b, type_field(t, i));
            }
            MPUT("}");
            break;
        }
        MPUT(mangle_prefix(t->decl));
        MPUT(".");
        MPUT(t->decl->name);
        if (t->nargs) {
            MPUT("[");
            for (int i = 0; i < t->nargs; i++) {
                if (i) MPUT(",");
                mangle_type_into(b, t->args[i]);
            }
            MPUT("]");
        }
        break;
    case TY_PTR:
        MPUT(t->mut ? "*mut " : "*");
        mangle_type_into(b, t->elem);
        break;
    case TY_SLICE:
        MPUT(t->mut ? "[]mut " : "[]");
        mangle_type_into(b, t->elem);
        break;
    case TY_ARRAY:
        MPUT(arena_printf("[%lld]", (long long)t->len));
        mangle_type_into(b, t->elem);
        break;
    case TY_ERRUNION:
        MPUT("!");
        mangle_type_into(b, t->elem);
        break;
    case TY_FN:
        MPUT("fn(");
        for (int i = 0; i < t->nparams; i++) {
            if (i) MPUT(",");
            mangle_type_into(b, t->params[i]);
        }
        MPUT(")");
        mangle_type_into(b, t->ret);
        break;
    default: MPUT(type_str(t)); break;
    }
#undef MPUT
}

static const char *mangle_type(Type *t) {
    CharVec b = {0};
    mangle_type_into(&b, t);
    vec_push(b, 0);
    const char *r = arena_strndup(b.data, strlen(b.data));
    free(b.data);
    return r;
}

// Owner.[A].name.[B], or test "name".
static const char *fn_display(Decl *d, Type **gargs, int n) {
    if (d->is_test) return arena_printf("test \"%s\"", d->test_name);
    CharVec b = {0};
#define PUTS(str)                                    \
    do {                                             \
        for (const char *q_ = (str); *q_; q_++) vec_push(b, *q_); \
    } while (0)
    int nowner = d->owner ? d->owner->gparams.len : 0;
    if (d->owner) {
        PUTS(d->owner->name);
        if (nowner) {
            PUTS(".[");
            for (int i = 0; i < nowner && i < n; i++) {
                if (i) PUTS(", ");
                PUTS(type_str(gargs[i]));
            }
            PUTS("]");
        }
        PUTS(".");
    }
    PUTS(d->name);
    int nown = d->kind == D_HOST_FN ? 0 : d->gparams.len;
    if (nown) {
        PUTS(".[");
        for (int i = n - nown; i < n; i++) {
            if (i > n - nown) PUTS(", ");
            PUTS(type_str(gargs[i]));
        }
        PUTS("]");
    }
#undef PUTS
    vec_push(b, 0);
    const char *r = arena_strndup(b.data, strlen(b.data));
    free(b.data);
    return r;
}

// The symbol of an exported function. On wasm (and in test binaries, which
// have their own main) `main` is renamed; the export name is an attribute.
static const char *export_symbol(Decl *d) {
    if (g_sanitize_main && !g_test_mode && !strcmp(d->name, "main")) return "axiom.asan.main";
    return (g_target.wasm || g_test_mode) && !strcmp(d->name, "main") ? "axiom.export.main" : d->name;
}

// Globals live in the link unit when units are compiled separately (their
// initializers may contain error codes, which only the link unit knows).
static LLVMValueRef global_ref(Decl *d) {
    Sym k = ptr_key(d);
    LLVMValueRef g = symmap_get(&global_vals, k);
    if (g) return g;
    Type *t = d->type;
    g = LLVMAddGlobal(mod, lty(t), arena_printf("%s.%s", mangle_prefix(d), d->name));
    LLVMSetAlignment(g, (unsigned)type_align(t));
    if (d->tls) {
        LLVMSetThreadLocal(g, true);
        LLVMSetThreadLocalMode(g, LLVMInitialExecTLSModel);
    }
    symmap_put(&global_vals, k, g);
    if (O->unit == CG_MODULE) return g;  // defined by the link unit
    if (O->unit == CG_WHOLE) LLVMSetLinkage(g, LLVMInternalLinkage);
    LLVMSetInitializer(g, gen_const(d->init, t));
    if (!d->mut) LLVMSetGlobalConstant(g, true);
    return g;
}

// Program-wide state shared by all units: defined (internal) in a whole
// program, declared by module units, defined (external) by the link unit.
static LLVMValueRef shared_global(LLVMTypeRef t, const char *name, LLVMValueRef init, unsigned addrspace) {
    LLVMValueRef g = addrspace ? LLVMAddGlobalInAddressSpace(mod, t, name, addrspace) : LLVMAddGlobal(mod, t, name);
    if (O->unit == CG_MODULE) return g;
    LLVMSetInitializer(g, init);
    if (O->unit == CG_WHOLE) LLVMSetLinkage(g, LLVMInternalLinkage);
    return g;
}

// Thread-locals on wasm, where a thread is a new instance sharing the memory:
// all of the program's thread-locals live in one block. The main thread uses
// axiom.tls.main; a thread start copies axiom.tls.init (the initial values)
// to the top of the new thread's stack and points the wasm global axiom.tls
// (null in the main thread) at it.
static LLVMTypeRef tls_ty;
static LLVMValueRef tls_main, tls_init, tls_ptr;
static Vec(Decl *) tls_decls;

static bool tls_globals(void) {
    if (tls_ty) return true;
    for (int i = 0; i < g_prog.globals.len; i++)
        if (g_prog.globals.data[i]->tls) vec_push(tls_decls, g_prog.globals.data[i]);
    int n = tls_decls.len;
    if (!n) return false;
    LLVMTypeRef *fs = NEWN(LLVMTypeRef, n);
    for (int k = 0; k < n; k++) fs[k] = lty(tls_decls.data[k]->type);
    tls_ty = LLVMStructTypeInContext(ctx, fs, (unsigned)n, false);
    LLVMValueRef init = NULL;
    if (O->unit != CG_MODULE) {
        LLVMValueRef *vs = NEWN(LLVMValueRef, n);
        for (int k = 0; k < n; k++) vs[k] = gen_const(tls_decls.data[k]->init, tls_decls.data[k]->type);
        init = LLVMConstNamedStruct(tls_ty, vs, (unsigned)n);
    }
    unsigned align = LLVMABIAlignmentOfType(TD, tls_ty);
    if (align < 8) align = 8;
    tls_main = shared_global(tls_ty, "axiom.tls.main", init, 0);
    LLVMSetAlignment(tls_main, align);
    tls_init = shared_global(tls_ty, "axiom.tls.init", init, 0);
    LLVMSetAlignment(tls_init, align);
    LLVMSetGlobalConstant(tls_init, true);
    tls_ptr = shared_global(T_ptr, "axiom.tls", LLVMConstNull(T_ptr), 1);
    return true;
}

static LLVMValueRef tls_addr(Decl *d) {
    tls_globals();
    unsigned field = 0;
    while (tls_decls.data[field] != d) field++;
    LLVMValueRef p = LLVMBuildLoad2(B, T_ptr, tls_ptr, "");
    LLVMValueRef base = LLVMBuildSelect(B, LLVMBuildIsNull(B, p, ""), tls_main, p, "");
    return LLVMBuildStructGEP2(B, tls_ty, base, field, "");
}

// The address of a global in the current thread.
static LLVMValueRef global_addr(Decl *d) {
    if (d->tls && g_target.wasm) return tls_addr(d);
    return global_ref(d);
}

// An error code: a constant, except in module units, where the codes are
// only known at link time (spec §11.1) and are loaded from the link unit.
static LLVMValueRef errcode_value(uint32_t code) {
    if (O->unit != CG_MODULE || code == 0 || (int)code > g_prog.error_names.len)
        return LLVMConstInt(T_i16, code, false);
    const char *name = arena_printf("axiom.errcode.%s", g_prog.error_names.data[code - 1]);
    LLVMValueRef g = LLVMGetNamedGlobal(mod, name);
    if (!g) {
        g = LLVMAddGlobal(mod, T_i16, name);
        LLVMSetGlobalConstant(g, true);
    }
    return LLVMBuildLoad2(B, T_i16, g, "");
}

static bool type_mentions_error(Type *t) {
    switch (t->kind) {
    case TY_ERROR:
    case TY_ERRUNION: return true;
    case TY_ARRAY: return type_mentions_error(t->elem);
    case TY_STRUCT:
        for (int i = 0; i < t->decl->fields.len; i++)
            if (type_mentions_error(type_field(t, i))) return true;
        return false;
    case TY_UNION: return true;  // conservatively
    default: return false;
    }
}

// ---------------------------------------------------------------------------
// Function instances
// ---------------------------------------------------------------------------
static void add_str_fn_attr(LLVMValueRef fn, const char *k, const char *v) {
    LLVMAddAttributeAtIndex(fn, LLVMAttributeFunctionIndex,
                            LLVMCreateStringAttribute(ctx, k, (unsigned)strlen(k), v, (unsigned)strlen(v)));
}

// A function whose extensions differ from the target's (a version of an
// @features or @multiversion function, or std/os/cpu.ax, compiled for the
// bare architecture) says so to LLVM.
static void set_fn_features(LLVMValueRef fn, uint64_t feats) {
    if (g_target.wasm || feats == g_features) return;
    add_str_fn_attr(fn, "target-features", llvm_features(feats));
    if (feats == 0) add_str_fn_attr(fn, "target-cpu", g_arch == ARCH_X64 ? "x86-64" : "generic");
}

static FnInst *get_inst(Decl *d, Type **gargs, int n) {
    CharVec key = {0};
    const char *p = arena_printf("%p", (void *)d);
    while (*p) vec_push(key, *p++);
    for (int i = 0; i < n; i++) {
        const char *s = arena_printf(",%d", gargs[i]->id);
        while (*s) vec_push(key, *s++);
    }
    Sym k = intern(key.data, (size_t)key.len);
    free(key.data);
    FnInst *fi = symmap_get(&inst_map, k);
    if (fi) return fi;
    fi = NEW(FnInst);
    fi->decl = d;
    fi->gargs = gargs;
    fi->ngargs = n;
    fi->key = k;
    fi->feats = fn_base_features(d);
    fi->dispatch = d->kind == D_FN && d->attr != ATTR_NONE;
    for (int i = 0; i < n; i++)
        if (gargs[i]->has_params) internal_error("generic instance with unsubstituted arguments");
    fi->fnty = n ? type_subst(d->type, d->all_gparams, gargs, n) : d->type;
    fi->llty = fn_lty(fi->fnty, &fi->sret);
    fi->display = fn_display(d, gargs, n);
    const char *name;
    if (d->kind == D_HOST_FN) {
        name = d->name;
    } else if (d->is_export) {
        // On wasm the symbol gets a private name and the export name comes
        // from the wasm-export-name attribute: LLVM treats a symbol called
        // `main` specially on wasm (it would add a second `main` export).
        // Test binaries bring their own `main` (the test driver).
        name = export_symbol(d);
    } else {
        CharVec nb = {0};
        const char *pre = mangle_prefix(d);
        const char *s = arena_printf("%s.%s%s%s", pre, d->owner ? d->owner->name : "", d->owner ? "." : "", d->name);
        while (*s) vec_push(nb, *s++);
        int nown = d->kind == D_HOST_FN ? 0 : d->gparams.len;
        if (nown) {
            const char *o = ".[";
            while (*o) vec_push(nb, *o++);
            for (int i = n - nown; i < n; i++) {
                const char *ts = mangle_type(gargs[i]);
                if (i > n - nown) {
                    vec_push(nb, ',');
                    vec_push(nb, ' ');
                }
                while (*ts) vec_push(nb, *ts++);
            }
            vec_push(nb, ']');
        }
        if (d->owner && d->owner->gparams.len) {
            const char *o = "@[";
            while (*o) vec_push(nb, *o++);
            for (int i = 0; i < d->owner->gparams.len; i++) {
                const char *ts = mangle_type(gargs[i]);
                if (i) vec_push(nb, ',');
                while (*ts) vec_push(nb, *ts++);
            }
            vec_push(nb, ']');
        }
        vec_push(nb, 0);
        name = arena_strndup(nb.data, strlen(nb.data));
        free(nb.data);
    }
    if (d->variadic) {
        unsigned np = LLVMCountParamTypes(fi->llty);
        LLVMTypeRef *pts = NEWN(LLVMTypeRef, np + 1);
        LLVMGetParamTypes(fi->llty, pts);
        fi->llty = LLVMFunctionType(LLVMGetReturnType(fi->llty), pts, np, true);
    }
    fi->ref_llty = fi->llty;
    if (d->kind == D_FN && d->is_inline && inline_sites_on()) {
        // trailing parameters: the callers' call-site records (see INL_SITES)
        unsigned np = LLVMCountParamTypes(fi->llty);
        LLVMTypeRef *pts = NEWN(LLVMTypeRef, np + INL_SITES);
        LLVMGetParamTypes(fi->llty, pts);
        for (int i = 0; i < INL_SITES; i++) pts[np + i] = T_ptr;
        fi->llty = LLVMFunctionType(LLVMGetReturnType(fi->llty), pts, np + INL_SITES, false);
        fi->ninl = INL_SITES;
    }
    LLVMValueRef existing = d->kind == D_HOST_FN ? LLVMGetNamedFunction(mod, name) : NULL;
    fi->fn = existing ? existing : LLVMAddFunction(mod, name, fi->llty);
    if (d->kind == D_FN && d->is_inline) {
        unsigned k = LLVMGetEnumAttributeKindForName("alwaysinline", 12);
        LLVMAddAttributeAtIndex(fi->fn, LLVMAttributeFunctionIndex, LLVMCreateEnumAttribute(ctx, k, 0));
    }
    if (d->kind == D_FN) set_fn_features(fi->fn, fi->feats);
    add_ext_attrs(fi->fn, fi->fnty, fi->sret, false);
    if (d->kind == D_HOST_FN) {
        if (d->extern_c) {
            // libraries: see collect_link_names
        } else if (g_target.wasm) {
            LLVMAddAttributeAtIndex(fi->fn, LLVMAttributeFunctionIndex,
                                    LLVMCreateStringAttribute(ctx, "wasm-import-module", 18, d->host_module,
                                                              (unsigned)strlen(d->host_module)));
            LLVMAddAttributeAtIndex(fi->fn, LLVMAttributeFunctionIndex,
                                    LLVMCreateStringAttribute(ctx, "wasm-import-name", 16, d->name,
                                                              (unsigned)strlen(d->name)));
        }
    } else {
        // Whole program: everything but exports is internal. Separate units:
        // a non-generic function is defined (externally visible) by its own
        // module's unit and only declared elsewhere; generic instances are
        // defined link-once by every unit that uses them.
        // Inline functions are defined (internal) by every unit that calls them.
        bool define = true;
        if (O->unit == CG_LINK) define = false;
        else if (O->unit == CG_MODULE && n == 0) define = d->module == O->module || d->is_inline;
        if ((O->unit == CG_WHOLE && !d->is_export) || d->is_inline) LLVMSetLinkage(fi->fn, LLVMInternalLinkage);
        else if (define && n > 0) LLVMSetLinkage(fi->fn, LLVMLinkOnceODRLinkage);
        bool asan_main = O->sanitize_address && !g_test_mode && !strcmp(d->name, "main");
        if (define && d->is_export && g_target.wasm && !asan_main)
            LLVMAddAttributeAtIndex(fi->fn, LLVMAttributeFunctionIndex,
                                    LLVMCreateStringAttribute(ctx, "wasm-export-name", 16, d->name,
                                                              (unsigned)strlen(d->name)));
        if (define) {
            if (pending_tail) pending_tail->next = fi;
            else pending_head = fi;
            pending_tail = fi;
        }
    }
    symmap_put(&inst_map, k, fi);
    return fi;
}

// A function as a value. An inline function taking its callers' call sites
// (INL_SITES) is called through a function with the ordinary signature,
// into which it is inlined.
static LLVMValueRef fn_ref(FnInst *fi) {
    if (!fi->ninl) return fi->fn;
    if (fi->ref) return fi->ref;
    size_t len;
    const char *name = LLVMGetValueName2(fi->fn, &len);
    fi->ref = LLVMAddFunction(mod, arena_printf("%s.ref", name), fi->ref_llty);
    LLVMSetLinkage(fi->ref, LLVMInternalLinkage);
    add_ext_attrs(fi->ref, fi->fnty, fi->sret, false);
    LLVMBasicBlockRef saved = LLVMGetInsertBlock(B);
    LLVMMetadataRef loc = dbg_suspend();
    LLVMPositionBuilderAtEnd(B, LLVMAppendBasicBlockInContext(ctx, fi->ref, "entry"));
    unsigned np = LLVMCountParams(fi->ref);
    LLVMValueRef *av = NEWN(LLVMValueRef, np + INL_SITES);
    for (unsigned i = 0; i < np; i++) av[i] = LLVMGetParam(fi->ref, i);
    for (int i = 0; i < fi->ninl; i++) av[np + i] = LLVMConstNull(T_ptr);
    LLVMValueRef r = LLVMBuildCall2(B, fi->llty, fi->fn, av, np + (unsigned)fi->ninl, "");
    add_ext_attrs(r, fi->fnty, fi->sret, true);
    if (LLVMGetTypeKind(LLVMGetReturnType(fi->ref_llty)) == LLVMVoidTypeKind) LLVMBuildRetVoid(B);
    else LLVMBuildRet(B, r);
    if (saved) LLVMPositionBuilderAtEnd(B, saved);
    else LLVMClearInsertionPosition(B);
    dbg_resume(loc);
    return fi->ref;
}

// C ABI detail: integers narrower than 32 bits and bool are passed and
// returned sign- or zero-extended. Marking every function (and indirect call)
// this way makes any Axiom function with a scalar signature callable from
// C, and C function pointers callable from Axiom (spec §14.5).
static void ext_attr(LLVMValueRef v, unsigned idx, Type *t, bool is_call) {
    const char *kind = NULL;
    t = ct(t);
    if (t->kind == TY_BOOL) kind = "zeroext";
    else if (t->kind == TY_INT && t->bits < 32) kind = t->is_signed ? "signext" : "zeroext";
    if (!kind) return;
    LLVMAttributeRef a = LLVMCreateEnumAttribute(ctx, LLVMGetEnumAttributeKindForName(kind, strlen(kind)), 0);
    if (is_call) LLVMAddCallSiteAttribute(v, idx, a);
    else LLVMAddAttributeAtIndex(v, idx, a);
}

static void add_ext_attrs(LLVMValueRef v, Type *fnty, bool sret, bool is_call) {
    if (!sret && fnty->ret->kind != TY_VOID) ext_attr(v, LLVMAttributeReturnIndex, fnty->ret, is_call);
    unsigned first = sret ? 2 : 1;
    for (int i = 0; i < fnty->nparams; i++)
        if (!by_ptr(fnty->params[i])) ext_attr(v, first + (unsigned)i, fnty->params[i], is_call);
}

static Type **subst_gargs(Type **g, int n) {
    if (!n) return NULL;
    Type **r = NEWN(Type *, n);
    for (int i = 0; i < n; i++) r[i] = ct(g[i]);
    return r;
}

// ---------------------------------------------------------------------------
// Defers and control flow
// ---------------------------------------------------------------------------
static void emit_defers(int down_to) {
    for (int i = F->defers.len - 1; i >= down_to; i--) {
        if (terminated()) return;
        Stmt *s = F->defers.data[i];
        int saved = F->defers.len;
        F->defers.len = i;  // a deferred statement runs with only the outer defers pending
        gen_stmt(s);
        F->defers.len = saved;
    }
}

static void emit_ret(LLVMValueRef v) {
    if (F->sret_ptr) {
        store(F->ret, v, F->sret_ptr, false);
        LLVMBuildRetVoid(B);
    } else if (F->ret->kind == TY_VOID) {
        LLVMBuildRetVoid(B);
    } else {
        LLVMBuildRet(B, v);
    }
}

static LLVMValueRef errunion_value(Type *eu, LLVMValueRef code, LLVMValueRef payload) {
    LLVMTypeRef t = lty(eu);
    LLVMValueRef v = LLVMGetUndef(t);
    v = LLVMBuildInsertValue(B, v, code, 0, "");
    if (eu->elem->kind != TY_VOID)
        v = LLVMBuildInsertValue(B, v, payload ? to_mem(eu->elem, payload) : LLVMConstNull(lty(eu->elem)), 1, "");
    return v;
}

static void gen_return(Expr *e) {
    LLVMValueRef v = NULL;
    if (e) {
        v = gen(e);
        if (terminated()) return;
        if (F->ret->kind == TY_ERRUNION) etrace_push(LLVMBuildExtractValue(B, v, 0, ""));
    } else if (F->ret->kind == TY_ERRUNION) {
        v = errunion_value(F->ret, LLVMConstInt(T_i16, 0, false), NULL);
    }
    emit_defers(0);
    if (terminated()) return;
    emit_ret(v);
    dead_block();
}

static void gen_break(bool is_break) {
    if (!F->loops.len) return;
    LoopCtx *lc = &vec_last(F->loops);
    emit_defers(lc->defer_height);
    LLVMBuildBr(B, is_break ? lc->brk : lc->cont);
    dead_block();
}

// ---------------------------------------------------------------------------
// Arithmetic
// ---------------------------------------------------------------------------
static LLVMValueRef int_min(LLVMTypeRef t) {
    unsigned bits = LLVMGetIntTypeWidth(t);
    return cint(t, -((i128)1 << (bits - 1)));
}
static LLVMValueRef int_max(LLVMTypeRef t, bool is_signed) {
    unsigned bits = LLVMGetIntTypeWidth(t);
    return is_signed ? cint(t, ((i128)1 << (bits - 1)) - 1) : cint(t, (i128)(((unsigned __int128)1 << bits) - 1));
}

// Overflow-checked arithmetic, expressed with plain instructions only.
// (The llvm.*.with.overflow intrinsics would force LLVM's slow SelectionDAG
// path at -O0 on wasm; at -O1+ LLVM re-forms the intrinsics from these
// patterns anyway.)
static void mul_wide_overflow(bool is_signed, LLVMValueRef l, LLVMValueRef r, LLVMValueRef *val, LLVMValueRef *ovf) {
    LLVMTypeRef t = LLVMTypeOf(l);
    unsigned bits = LLVMGetIntTypeWidth(t);
    LLVMTypeRef w = LLVMIntTypeInContext(ctx, bits * 2);
    LLVMValueRef lw = is_signed ? LLVMBuildSExt(B, l, w, "") : LLVMBuildZExt(B, l, w, "");
    LLVMValueRef rw = is_signed ? LLVMBuildSExt(B, r, w, "") : LLVMBuildZExt(B, r, w, "");
    LLVMValueRef p = LLVMBuildMul(B, lw, rw, "");
    *val = LLVMBuildTrunc(B, p, t, "");
    LLVMValueRef back = is_signed ? LLVMBuildSExt(B, *val, w, "") : LLVMBuildZExt(B, *val, w, "");
    *ovf = LLVMBuildICmp(B, LLVMIntNE, back, p, "");
}

// Bits of `v` that can be nonzero (unsigned), or that can differ from the
// sign bit (signed), if `v` is a zero or sign extension; 0 if unknown.
static unsigned ext_bits(LLVMValueRef v, bool is_signed) {
    if (!LLVMIsAZExtInst(v) && !LLVMIsASExtInst(v)) return 0;
    unsigned from = LLVMGetIntTypeWidth(LLVMTypeOf(LLVMGetOperand(v, 0)));
    if (LLVMIsAZExtInst(v)) return is_signed ? from + 1 : from;  // signed: the (zero) sign bit
    return is_signed ? from : 0;
}

// 64-bit multiply. Operands extended from at most half the width cannot
// overflow. On native targets, and on wasm at -O1 and above, the overflow
// intrinsics give it from one multiply (x86: `mul`/`imul` and `seto`).
// At -O0 on wasm they would force LLVM's slow SelectionDAG path, so there
// overflow is detected by dividing back (no 128-bit multiply: freestanding
// wasm has no compiler-rt). With a constant operand the other is compared
// with the quotients of the limits.
static void mul64_overflow(bool is_signed, LLVMValueRef l, LLVMValueRef r, LLVMValueRef *val, LLVMValueRef *ovf) {
    LLVMTypeRef t = LLVMTypeOf(l);
    LLVMValueRef zero = LLVMConstNull(t), one = LLVMConstInt(t, 1, false);
    unsigned half = LLVMGetIntTypeWidth(t) / 2;
    unsigned lb = ext_bits(l, is_signed), rb = ext_bits(r, is_signed);
    if (lb && rb && lb <= half && rb <= half) {
        // |l| < 2^lb and |r| < 2^rb (signed: |x| <= 2^(b-1)), so the product fits
        *val = LLVMBuildMul(B, l, r, "");
        *ovf = LLVMConstInt(T_i1, 0, false);
        return;
    }
    if (!LLVMIsAConstantInt(l) && !LLVMIsAConstantInt(r) && (!g_target.wasm || O->opt_level > 0)) {
        LLVMValueRef args[2] = {l, r};
        LLVMValueRef res = intrinsic(is_signed ? "llvm.smul.with.overflow" : "llvm.umul.with.overflow", &t, 1, args, 2);
        *val = LLVMBuildExtractValue(B, res, 0, "");
        *ovf = LLVMBuildExtractValue(B, res, 1, "");
        return;
    }
    LLVMValueRef prod = LLVMBuildMul(B, l, r, "");
    if (LLVMIsAConstantInt(l) || LLVMIsAConstantInt(r)) {
        LLVMValueRef kc = LLVMIsAConstantInt(r) ? r : l, x = kc == r ? l : r;
        unsigned bits = LLVMGetIntTypeWidth(t);
        *val = prod;
        if (!is_signed) {
            unsigned __int128 k = (unsigned long long)LLVMConstIntGetZExtValue(kc);
            unsigned __int128 mx = ((unsigned __int128)1 << bits) - 1;
            *ovf = k == 0 ? LLVMConstInt(T_i1, 0, false) : LLVMBuildICmp(B, LLVMIntUGT, x, cint(t, (i128)(mx / k)), "");
            return;
        }
        i128 k = (i128)LLVMConstIntGetSExtValue(kc);
        i128 mx = ((i128)1 << (bits - 1)) - 1, mn = -mx - 1;
        if (k == 0) *ovf = LLVMConstInt(T_i1, 0, false);
        else if (k == -1) *ovf = LLVMBuildICmp(B, LLVMIntEQ, x, cint(t, mn), "");
        else if (k > 0)  // (division truncates: floor(mx / k), ceil(mn / k))
            *ovf = LLVMBuildOr(B, LLVMBuildICmp(B, LLVMIntSGT, x, cint(t, mx / k), ""),
                               LLVMBuildICmp(B, LLVMIntSLT, x, cint(t, mn / k), ""), "");
        else
            *ovf = LLVMBuildOr(B, LLVMBuildICmp(B, LLVMIntSLT, x, cint(t, mx / k), ""),
                               LLVMBuildICmp(B, LLVMIntSGT, x, cint(t, mn / k), ""), "");
        return;
    }
    LLVMValueRef lz = LLVMBuildICmp(B, LLVMIntEQ, l, zero, "");
    if (!is_signed) {
        LLVMValueRef div = LLVMBuildSelect(B, lz, one, l, "");
        LLVMValueRef back = LLVMBuildUDiv(B, prod, div, "");
        *ovf = LLVMBuildAnd(B, LLVMBuildNot(B, lz, ""), LLVMBuildICmp(B, LLVMIntNE, back, r, ""), "");
    } else {
        LLVMValueRef lm1 = LLVMBuildICmp(B, LLVMIntEQ, l, LLVMConstAllOnes(t), "");
        LLVMValueRef safe = LLVMBuildOr(B, lz, lm1, "");
        LLVMValueRef div = LLVMBuildSelect(B, safe, one, l, "");
        LLVMValueRef back = LLVMBuildSDiv(B, prod, div, "");
        LLVMValueRef general = LLVMBuildAnd(B, LLVMBuildNot(B, safe, ""), LLVMBuildICmp(B, LLVMIntNE, back, r, ""), "");
        LLVMValueRef m1case = LLVMBuildAnd(B, lm1, LLVMBuildICmp(B, LLVMIntEQ, r, int_min(t), ""), "");
        *ovf = LLVMBuildOr(B, general, m1case, "");
    }
    *val = prod;
}

static void op_with_overflow(const char *base, bool is_signed, LLVMValueRef l, LLVMValueRef r, LLVMValueRef *val,
                             LLVMValueRef *ovf) {
    LLVMTypeRef t = LLVMTypeOf(l);
    LLVMValueRef zero = LLVMConstNull(t);
    if (!strcmp(base, "add")) {
        *val = LLVMBuildAdd(B, l, r, "");
        if (is_signed) {
            LLVMValueRef x = LLVMBuildAnd(B, LLVMBuildXor(B, l, *val, ""), LLVMBuildXor(B, r, *val, ""), "");
            *ovf = LLVMBuildICmp(B, LLVMIntSLT, x, zero, "");
        } else {
            *ovf = LLVMBuildICmp(B, LLVMIntULT, *val, l, "");
        }
        return;
    }
    if (!strcmp(base, "sub")) {
        *val = LLVMBuildSub(B, l, r, "");
        if (is_signed) {
            LLVMValueRef x = LLVMBuildAnd(B, LLVMBuildXor(B, l, r, ""), LLVMBuildXor(B, l, *val, ""), "");
            *ovf = LLVMBuildICmp(B, LLVMIntSLT, x, zero, "");
        } else {
            *ovf = LLVMBuildICmp(B, LLVMIntULT, l, r, "");
        }
        return;
    }
    if (LLVMGetIntTypeWidth(t) <= 32) mul_wide_overflow(is_signed, l, r, val, ovf);
    else mul64_overflow(is_signed, l, r, val, ovf);
}

static LLVMValueRef saturated(bool is_signed, LLVMTypeRef t, LLVMValueRef neg_direction) {
    if (!is_signed) return neg_direction;  // caller passes the unsigned bound directly
    return LLVMBuildSelect(B, neg_direction, int_min(t), int_max(t, true), "");
}

static LLVMValueRef overflow_op(const char *base, bool is_signed, LLVMValueRef l, LLVMValueRef r, bool mul) {
    // base: "add", "sub", "mul"
    LLVMTypeRef t = LLVMTypeOf(l);
    LLVMValueRef val, ovf;
    op_with_overflow(base, is_signed, l, r, &val, &ovf);
    if (!O->overflow_saturate) {
        check_or_trap(LLVMBuildNot(B, ovf, ""), T_OVERFLOW);
        return val;
    }
    LLVMValueRef sat;
    LLVMValueRef zero = LLVMConstNull(t);
    if (!strcmp(base, "mul")) {
        sat = is_signed ? saturated(true, t, LLVMBuildICmp(B, LLVMIntSLT, LLVMBuildXor(B, l, r, ""), zero, ""))
                        : int_max(t, false);
    } else if (!strcmp(base, "add")) {
        sat = is_signed ? saturated(true, t, LLVMBuildICmp(B, LLVMIntSLT, l, zero, "")) : int_max(t, false);
    } else {
        sat = is_signed ? saturated(true, t, LLVMBuildICmp(B, LLVMIntSLT, l, zero, "")) : zero;
    }
    return LLVMBuildSelect(B, ovf, sat, val, "");
}

// Shift with mathematical semantics for every count (spec §7.6).
static LLVMValueRef gen_shift(int op, Type *t, LLVMValueRef x, LLVMValueRef k, Type *kt) {
    Type *lane = t->kind == TY_VEC ? t->elem : t;
    int n = lane->bits;
    bool arith = op == TK_SHR && lane->is_signed;
    if (kt && kt->kind == TY_VEC) {
        // a count per lane, of the lanes' width (vpsllvd, ushl, ...)
        LLVMTypeRef vt = LLVMTypeOf(x);
        LLVMValueRef cn[64], cf[64];
        for (int i = 0; i < t->lanes; i++) {
            cn[i] = LLVMConstInt(LLVMGetElementType(vt), (unsigned long long)n, false);
            cf[i] = LLVMConstInt(LLVMGetElementType(vt), arith ? (unsigned long long)(n - 1) : 0, false);
        }
        LLVMValueRef vge = LLVMBuildICmp(B, LLVMIntUGE, k, LLVMConstVector(cn, (unsigned)t->lanes), "");
        LLVMValueRef vk = LLVMBuildSelect(B, vge, LLVMConstVector(cf, (unsigned)t->lanes), k, "");
        if (arith) return LLVMBuildAShr(B, x, vk, "");
        LLVMValueRef vr = op == TK_SHL ? LLVMBuildShl(B, x, vk, "") : LLVMBuildLShr(B, x, vk, "");
        return LLVMBuildSelect(B, vge, LLVMConstNull(vt), vr, "");
    }
    LLVMTypeRef kty = LLVMTypeOf(k);
    LLVMValueRef ge = LLVMBuildICmp(B, LLVMIntUGE, k, LLVMConstInt(kty, (unsigned long long)n, false), "");
    LLVMTypeRef laneTy = LLVMIntTypeInContext(ctx, (unsigned)n);
    LLVMValueRef kc = LLVMBuildSelect(B, ge, LLVMConstInt(kty, arith ? (unsigned long long)(n - 1) : 0, false), k, "");
    unsigned kb = LLVMGetIntTypeWidth(kty);
    if (kb > (unsigned)n) kc = LLVMBuildTrunc(B, kc, laneTy, "");
    else if (kb < (unsigned)n) kc = LLVMBuildZExt(B, kc, laneTy, "");
    if (t->kind == TY_VEC) {
        LLVMTypeRef vt = LLVMTypeOf(x);
        LLVMValueRef v = LLVMBuildInsertElement(B, LLVMGetUndef(vt), kc, LLVMConstInt(T_i32, 0, false), "");
        LLVMValueRef mask = LLVMConstNull(LLVMVectorType(T_i32, (unsigned)t->lanes));
        kc = LLVMBuildShuffleVector(B, v, LLVMGetUndef(vt), mask, "");
    }
    LLVMValueRef r;
    if (op == TK_SHL) r = LLVMBuildShl(B, x, kc, "");
    else if (arith) return LLVMBuildAShr(B, x, kc, "");
    else r = LLVMBuildLShr(B, x, kc, "");
    return LLVMBuildSelect(B, ge, LLVMConstNull(LLVMTypeOf(x)), r, "");
}

static LLVMValueRef gen_binop(int op, Type *t, LLVMValueRef l, LLVMValueRef r, Type *rt) {
    bool is_float = t->kind == TY_FLOAT || ty_is_vec_float(t);
    bool is_vec = t->kind == TY_VEC || t->kind == TY_V128;
    bool sgn = t->kind == TY_INT ? t->is_signed : (t->kind == TY_VEC ? t->elem->is_signed : false);
    switch (op) {
    case TK_PLUS:
        if (is_float) return LLVMBuildFAdd(B, l, r, "");
        return overflow_op("add", sgn, l, r, false);
    case TK_MINUS:
        if (is_float) return LLVMBuildFSub(B, l, r, "");
        return overflow_op("sub", sgn, l, r, false);
    case TK_STAR:
        if (is_float) return LLVMBuildFMul(B, l, r, "");
        return overflow_op("mul", sgn, l, r, true);
    case TK_SLASH: {
        if (is_float) return LLVMBuildFDiv(B, l, r, "");
        LLVMTypeRef ty = LLVMTypeOf(l);
        check_or_trap(LLVMBuildICmp(B, LLVMIntNE, r, LLVMConstNull(ty), ""), T_DIVZERO);
        if (!sgn) return LLVMBuildUDiv(B, l, r, "");
        LLVMValueRef ovf = LLVMBuildAnd(B, LLVMBuildICmp(B, LLVMIntEQ, l, int_min(ty), ""),
                                        LLVMBuildICmp(B, LLVMIntEQ, r, LLVMConstAllOnes(ty), ""), "");
        if (!O->overflow_saturate) {
            check_or_trap(LLVMBuildNot(B, ovf, ""), T_OVERFLOW);
            return LLVMBuildSDiv(B, l, r, "");
        }
        LLVMValueRef r2 = LLVMBuildSelect(B, ovf, LLVMConstInt(ty, 1, false), r, "");
        LLVMValueRef q = LLVMBuildSDiv(B, l, r2, "");
        return LLVMBuildSelect(B, ovf, int_max(ty, true), q, "");
    }
    case TK_PERCENT: {
        LLVMTypeRef ty = LLVMTypeOf(l);
        check_or_trap(LLVMBuildICmp(B, LLVMIntNE, r, LLVMConstNull(ty), ""), T_DIVZERO);
        if (!sgn) return LLVMBuildURem(B, l, r, "");
        LLVMValueRef m1 = LLVMBuildICmp(B, LLVMIntEQ, r, LLVMConstAllOnes(ty), "");
        LLVMValueRef r2 = LLVMBuildSelect(B, m1, LLVMConstInt(ty, 1, false), r, "");
        return LLVMBuildSRem(B, l, r2, "");
    }
    case TK_PLUS_PCT: return LLVMBuildAdd(B, l, r, "");
    case TK_MINUS_PCT: return LLVMBuildSub(B, l, r, "");
    case TK_STAR_PCT: return LLVMBuildMul(B, l, r, "");
    case TK_AMP: return LLVMBuildAnd(B, l, r, "");
    case TK_PIPE: return LLVMBuildOr(B, l, r, "");
    case TK_CARET: return LLVMBuildXor(B, l, r, "");
    case TK_SHL:
    case TK_SHR: return gen_shift(op, t, l, r, rt);
    default: break;
    }
    // comparisons
    if (is_float) {
        LLVMRealPredicate p;
        switch (op) {
        case TK_EQEQ: p = LLVMRealOEQ; break;
        case TK_NE: p = LLVMRealUNE; break;
        case TK_LT: p = LLVMRealOLT; break;
        case TK_LE: p = LLVMRealOLE; break;
        case TK_GT: p = LLVMRealOGT; break;
        default: p = LLVMRealOGE; break;
        }
        return LLVMBuildFCmp(B, p, l, r, "");
    }
    (void)is_vec;
    LLVMIntPredicate p;
    switch (op) {
    case TK_EQEQ: p = LLVMIntEQ; break;
    case TK_NE: p = LLVMIntNE; break;
    case TK_LT: p = sgn ? LLVMIntSLT : LLVMIntULT; break;
    case TK_LE: p = sgn ? LLVMIntSLE : LLVMIntULE; break;
    case TK_GT: p = sgn ? LLVMIntSGT : LLVMIntUGT; break;
    default: p = sgn ? LLVMIntSGE : LLVMIntUGE; break;
    }
    return LLVMBuildICmp(B, p, l, r, "");
}

// ---------------------------------------------------------------------------
// Places
// ---------------------------------------------------------------------------
static LLVMValueRef addr_or_spill(Expr *e) {
    if (is_place(e)) return gen_addr(e);
    Type *t = ct(e->type);
    return spill(t, gen(e));
}

// Is e a place whose storage may be misaligned (reached through a pointer)?
static bool via_pointer(Expr *e) {
    switch (e->kind) {
    case E_DEREF: return true;
    case E_FIELD: return e->self_mode == SELF_DEREF || (e->op == FK_STRUCT && via_pointer(e->a));
    case E_INDEX: return e->op != IDX_ARRAY || via_pointer(e->a);
    default: return false;
    }
}

static LLVMValueRef array_len_value(Type *arr) { return usz(arr->len); }

static bool asan_on(void);
static void asan_check(LLVMValueRef addr, LLVMValueRef size);

// --sanitize=address: a place reached through a pointer or slice is checked
// where it is loaded or stored (the whole value; not the aggregates on the
// way to it).
static LLVMValueRef asan_place(Expr *e, LLVMValueRef addr) {
    if (asan_on() && via_pointer(e)) asan_check(addr, usz(type_size(ct(e->type))));
    return addr;
}

static LLVMValueRef gen_addr_inner(Expr *e) {
    switch (e->kind) {
    case E_IDENT:
        if (e->local) {
            return local_ptr(e->local);
        }
        if (e->decl && e->decl->kind == D_GLOBAL) return global_addr(e->decl);
        break;
    case E_DEREF: return gen(e->a);
    case E_FIELD: {
        if (e->op == FK_DECL && e->decl && e->decl->kind == D_GLOBAL) return global_addr(e->decl);
        Type *at = ct(e->a->type);
        LLVMValueRef base;
        Type *st;
        if (e->self_mode == SELF_DEREF) {
            base = gen(e->a);
            st = at->elem;
        } else {
            base = addr_or_spill(e->a);
            st = at;
        }
        return LLVMBuildStructGEP2(B, lty(st), base, (unsigned)e->index, "");
    }
    case E_INDEX: {
        Type *at = ct(e->a->type);
        if (e->op == IDX_SLICE) {
            LLVMValueRef s = gen(e->a);
            LLVMValueRef idx = gen(e->b);
            LLVMValueRef ptr = LLVMBuildExtractValue(B, s, 0, "");
            LLVMValueRef len = LLVMBuildExtractValue(B, s, 1, "");
            check_or_trap(LLVMBuildICmp(B, LLVMIntULT, idx, len, ""), T_BOUNDS);
            // in bounds (checked): the address cannot wrap, so a constant part
            // of the index can fold into the access (wasm's offset=)
            return LLVMBuildInBoundsGEP2(B, lty(at->elem), ptr, &idx, 1, "");
        }
        Type *arr = e->op == IDX_PTR_ARRAY ? at->elem : at;
        LLVMValueRef base = e->op == IDX_PTR_ARRAY ? gen(e->a) : addr_or_spill(e->a);
        LLVMValueRef idx = gen(e->b);
        check_or_trap(LLVMBuildICmp(B, LLVMIntULT, idx, array_len_value(arr), ""), T_BOUNDS);
        LLVMValueRef ix[2] = {usz(0), idx};
        return LLVMBuildInBoundsGEP2(B, lty(arr), base, ix, 2, "");
    }
    default: break;
    }
    // not a place: materialize
    Type *t = ct(e->type);
    return spill(t, gen(e));
}

// ---------------------------------------------------------------------------
// Calls
// ---------------------------------------------------------------------------
// `ninl`: the callee is an inline function taking its callers' call sites.
static LLVMValueRef emit_call(LLVMTypeRef fty, LLVMValueRef fn, Type *fnty, bool sret, LLVMValueRef *argv, int argc,
                              Type **argt, const char *callee, int ninl) {
    int n = argc + (sret ? 1 : 0);
    LLVMValueRef *av = NEWN(LLVMValueRef, n + ninl + 1);
    int k = 0;
    LLVMValueRef ret_tmp = NULL;
    if (sret) {
        ret_tmp = temp_alloca(fnty->ret);
        av[k++] = ret_tmp;
    }
    for (int i = 0; i < argc; i++) av[k++] = by_ptr(argt[i]) ? spill(argt[i], argv[i]) : argv[i];
    LLVMValueRef r;
    if (ninl) {
        // this call's record, then the caller's own (if it is inline too);
        // one that does not fit is pushed around the call
        av[k++] = site_record_ex(SITE_CALL, g_cur_span, callee);
        for (int i = 0; i < ninl - 1; i++) av[k++] = F && i < F->ninl ? F->inl[i] : LLVMConstNull(T_ptr);
        LLVMValueRef extra = F && F->ninl >= ninl ? F->inl[ninl - 1] : NULL, ss = NULL;
        if (extra) ss_push_site(ss = ss_ptr(), extra);
        r = LLVMBuildCall2(B, fty, fn, av, (unsigned)(n + ninl), "");
        if (extra) {
            LLVMBasicBlockRef doit = new_bb("ss.inline.pop"), next = new_bb("ss.inline.popped");
            LLVMBuildCondBr(B, LLVMBuildIsNotNull(B, extra, ""), doit, next);
            LLVMPositionBuilderAtEnd(B, doit);
            LLVMValueRef dp = ss_field(ss, 0);
            ss_store(LLVMBuildSub(B, LLVMBuildLoad2(B, T_i32, dp, ""), LLVMConstInt(T_i32, 1, false), ""), dp);
            LLVMBuildBr(B, next);
            LLVMPositionBuilderAtEnd(B, next);
        }
    } else {
        SsFrame sf = ss_push(callee);
        r = LLVMBuildCall2(B, fty, fn, av, (unsigned)n, "");
        ss_pop(sf);
    }
    add_ext_attrs(r, fnty, sret, true);
    if (sret) return load(fnty->ret, ret_tmp, false);
    if (fnty->ret->kind == TY_VOID) return NULL;
    return r;
}

// A function of runtime/rt.ax, which is loaded into every WebAssembly build:
// software versions of the operations WebAssembly has no instruction for
// (@rem, and @fma, whose libm calls would have nothing to link against).
static FnInst *rt_fn(const char *name) {
    for (int i = 0; i < g_prog.fns.len; i++) {
        Decl *d = g_prog.fns.data[i];
        if (strcmp(d->name, name) != 0 || !d->module) continue;
        const char *f = g_files.data[d->module->file->id]->display;
        size_t n = strlen(f);
        if (n >= 13 && !strcmp(f + n - 13, "runtime/rt.ax")) return get_inst(d, NULL, 0);
    }
    fprintf(stderr, "axiom: internal error: runtime function %s not found (is runtime/rt.ax missing?)\n", name);
    exit(3);
}

static bool is_f32_type(LLVMTypeRef t) {
    if (LLVMGetTypeKind(t) == LLVMVectorTypeKind) t = LLVMGetElementType(t);
    return LLVMGetTypeKind(t) == LLVMFloatTypeKind;
}

// --sanitize=address (spec §16.8): an access of `size` bytes at `addr` is
// checked against std/mem.ax's shadow memory (mem.asan_check), trapping with
// T0012 when it touches memory that is not allocated. The allocator itself
// and the runtime are not checked.
static bool asan_on(void) {
    if (!O->sanitize_address || !F || !F->decl || !F->decl->module) return false;
    const char *f = g_files.data[F->decl->module->file->id]->display;
    return strcmp(f, "std/mem.ax") != 0 && strncmp(f, "runtime/", 8) != 0;
}

static FnInst *mem_fn(const char *name) {
    for (int i = 0; i < g_prog.fns.len; i++) {
        Decl *d = g_prog.fns.data[i];
        if (strcmp(d->name, name) != 0 || !d->module) continue;
        if (!strcmp(g_files.data[d->module->file->id]->display, "std/mem.ax")) return get_inst(d, NULL, 0);
    }
    fprintf(stderr, "axiom: internal error: std/mem.ax has no %s (is --std right?)\n", name);
    exit(3);
}

static LLVMValueRef mem_global(const char *name) {
    for (int i = 0; i < g_prog.globals.len; i++) {
        Decl *d = g_prog.globals.data[i];
        if (strcmp(d->name, name) != 0 || !d->module) continue;
        if (!strcmp(g_files.data[d->module->file->id]->display, "std/mem.ax")) return global_addr(d);
    }
    fprintf(stderr, "axiom: internal error: std/mem.ax has no %s (is --std right?)\n", name);
    exit(3);
}

// The check is inline for the common cases: an access of at most 8 bytes
// within one granule of tracked (heap) memory whose shadow byte is 0, or an
// access within one 64 KiB page that the sanitizer does not track (stack,
// globals). It walks std/mem.ax's shadow tables (as_base, as_l1, then the
// page's table and its shadow bytes) and calls mem.asan_check, which
// reports, only when that fails.
static void asan_check(LLVMValueRef addr, LLVMValueRef size) {
    if (!asan_on()) return;
    LLVMValueRef a = LLVMBuildPtrToInt(B, addr, T_isz, "");
    LLVMValueRef sz = size;
    LLVMBasicBlockRef l1_bb = new_bb("asan.l1"), l2_bb = new_bb("asan.l2"), sh_bb = new_bb("asan.shadow");
    LLVMBasicBlockRef untracked = new_bb("asan.untracked"), slow = new_bb("asan.slow"), done = new_bb("asan.ok");
    LLVMValueRef base = LLVMBuildLoad2(B, T_isz, mem_global("as_base"), "");
    LLVMValueRef rel = LLVMBuildSub(B, LLVMBuildLShr(B, a, usz(16), ""), base, "");
    LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntULT, rel, usz(1 << 24), ""), l1_bb, untracked);
    // the page's table
    LLVMPositionBuilderAtEnd(B, l1_bb);
    LLVMValueRef l1 = mem_global("as_l1");
    LLVMValueRef idx[2] = {usz(0), LLVMBuildLShr(B, rel, usz(12), "")};
    LLVMValueRef l2 = LLVMBuildLoad2(B, T_isz, LLVMBuildInBoundsGEP2(B, LLVMArrayType(T_isz, 4096), l1, idx, 2, ""), "");
    LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntNE, l2, usz(0), ""), l2_bb, untracked);
    // the page's shadow
    LLVMPositionBuilderAtEnd(B, l2_bb);
    LLVMValueRef slot = LLVMBuildAdd(B, l2, LLVMBuildMul(B, LLVMBuildAnd(B, rel, usz(4095), ""), usz(8), ""), "");
    LLVMValueRef sp64 = LLVMBuildLoad2(B, T_i64, LLVMBuildIntToPtr(B, slot, T_ptr, ""), "");
    LLVMValueRef sp = LLVMBuildTruncOrBitCast(B, sp64, T_isz, "");
    LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntNE, sp, usz(0), ""), sh_bb, untracked);
    // the granule: all 8 bytes usable, and the access within it
    LLVMPositionBuilderAtEnd(B, sh_bb);
    LLVMValueRef sa = LLVMBuildAdd(B, sp, LLVMBuildLShr(B, LLVMBuildAnd(B, a, usz(0xffff), ""), usz(3), ""), "");
    LLVMValueRef sb = LLVMBuildLoad2(B, T_i8, LLVMBuildIntToPtr(B, sa, T_ptr, ""), "");
    LLVMValueRef in_granule = LLVMBuildAnd(B, LLVMBuildICmp(B, LLVMIntULE, sz, usz(8), ""),
        LLVMBuildICmp(B, LLVMIntULE, LLVMBuildAdd(B, LLVMBuildAnd(B, a, usz(7), ""), sz, ""), usz(8), ""), "");
    LLVMValueRef ok1 = LLVMBuildAnd(B, LLVMBuildICmp(B, LLVMIntEQ, sb, LLVMConstInt(T_i8, 0, false), ""), in_granule, "");
    LLVMBuildCondBr(B, ok1, done, slow);
    // memory the sanitizer does not track: fine within one page
    LLVMPositionBuilderAtEnd(B, untracked);
    LLVMValueRef in_page = LLVMBuildAnd(B, LLVMBuildICmp(B, LLVMIntULE, sz, usz(0x10000), ""),
        LLVMBuildICmp(B, LLVMIntULE, LLVMBuildAdd(B, LLVMBuildAnd(B, a, usz(0xffff), ""), sz, ""), usz(0x10000), ""), "");
    LLVMBuildCondBr(B, in_page, done, slow);
    // everything else: the full check, which reports
    LLVMPositionBuilderAtEnd(B, slow);
    FnInst *fi = mem_fn("asan_check");
    LLVMValueRef args[2] = {a, sz};
    LLVMValueRef ok = emit_call(fi->llty, fi->fn, fi->fnty, false, args, 2, fi->fnty->params, NULL, fi->ninl);
    check_or_trap(LLVMBuildICmp(B, LLVMIntNE, ok, LLVMConstInt(LLVMTypeOf(ok), 0, false), ""), T_MEMORY);
    LLVMBuildBr(B, done);
    LLVMPositionBuilderAtEnd(B, done);
}

// Checks the memory of a slice value (all of its elements).
static void asan_check_slice(LLVMValueRef s, Type *elem) {
    if (!asan_on()) return;
    LLVMValueRef len = LLVMBuildExtractValue(B, s, 1, "");
    asan_check(LLVMBuildExtractValue(B, s, 0, ""), LLVMBuildMul(B, len, usz(type_size(elem)), ""));
}

// Calls rt function `name` on scalar arguments, or lane by lane on vectors.
static LLVMValueRef rt_call_lanes(const char *name, LLVMValueRef *args, int n) {
    FnInst *fi = rt_fn(name);
    LLVMTypeRef t = LLVMTypeOf(args[0]);
    if (LLVMGetTypeKind(t) != LLVMVectorTypeKind) {
        return emit_call(fi->llty, fi->fn, fi->fnty, false, args, n, fi->fnty->params, NULL, fi->ninl);
    }
    unsigned lanes = LLVMGetVectorSize(t);
    LLVMValueRef r = LLVMGetUndef(t);
    for (unsigned l = 0; l < lanes; l++) {
        LLVMValueRef ls[3];
        LLVMValueRef ix = LLVMConstInt(T_i32, l, false);
        for (int k = 0; k < n; k++) ls[k] = LLVMBuildExtractElement(B, args[k], ix, "");
        LLVMValueRef v = emit_call(fi->llty, fi->fn, fi->fnty, false, ls, n, fi->fnty->params, NULL, fi->ninl);
        r = LLVMBuildInsertElement(B, r, v, ix, "");
    }
    return r;
}

static LLVMValueRef gen_call(Expr *e) {
    if (e->op == CALLK_VARIANT) {
        Type *ut = ct(e->tval);
        LLVMValueRef tmp = temp_alloca(ut);
        LLVMValueRef tagp = LLVMBuildStructGEP2(B, lty(ut), tmp, 0, "");
        LLVMValueRef payload = gen(e->args.data[0]);
        LLVMBuildStore(B, LLVMConstInt(lty(union_tag_type(ut)), (unsigned long long)e->index, false), tagp);
        Type *pt = type_variant(ut, e->index);
        LLVMValueRef pp = LLVMBuildStructGEP2(B, lty(ut), tmp, 1, "");
        store(pt, payload, pp, true);
        return load(ut, tmp, false);
    }
    if (e->op == CALLK_INDIRECT) {
        Type *ft = ct(e->a->type);
        LLVMValueRef fp = gen(e->a);
        int n = e->args.len;
        LLVMValueRef *av = NEWN(LLVMValueRef, n + 1);
        for (int i = 0; i < n; i++) av[i] = gen(e->args.data[i]);
        bool sret;
        LLVMTypeRef fty = fn_lty(ft, &sret);
        return emit_call(fty, fp, ft, sret, av, n, ft->params, NULL, 0);
    }
    Decl *d = e->decl;
    FnInst *fi = get_inst(d, subst_gargs(e->gargs, e->ngargs), e->ngargs);
    int n = e->args.len + (e->b ? 1 : 0);
    LLVMValueRef *av = NEWN(LLVMValueRef, n + 1);
    int k = 0;
    if (e->b) {
        Expr *recv = e->b;
        switch (e->self_mode) {
        case SELF_VALUE: av[k++] = gen(recv); break;
        case SELF_DEREF: {
            Type *pt = ct(recv->type);
            av[k++] = load(pt->elem, gen(recv), true);
            break;
        }
        case SELF_ADDR: av[k++] = gen_addr(recv); break;
        case SELF_PASS: av[k++] = gen(recv); break;
        default: av[k++] = gen(recv); break;
        }
    }
    for (int i = 0; i < e->args.len; i++) av[k++] = gen(e->args.data[i]);
    Type **argt = fi->fnty->params;
    if (d->variadic) {  // types of the extra (promoted) arguments
        argt = NEWN(Type *, n + 1);
        for (int i = 0; i < n; i++)
            argt[i] = i < fi->fnty->nparams ? fi->fnty->params[i] : ct(e->args.data[i - (e->b ? 1 : 0)]->type);
    }
    return emit_call(fi->llty, fi->fn, fi->fnty, fi->sret, av, n, argt, fi->display, fi->ninl);
}

// ---------------------------------------------------------------------------
// catch / try
// ---------------------------------------------------------------------------
static LLVMValueRef gen_try(Expr *e) {
    Type *eu = ct(e->a->type);
    LLVMValueRef v = gen(e->a);
    LLVMValueRef code = LLVMBuildExtractValue(B, v, 0, "");
    LLVMValueRef is_err = LLVMBuildICmp(B, LLVMIntNE, code, LLVMConstInt(T_i16, 0, false), "");
    LLVMBasicBlockRef err_bb = new_bb("try.err"), ok_bb = new_bb("try.ok");
    LLVMBuildCondBr(B, is_err, err_bb, ok_bb);
    LLVMPositionBuilderAtEnd(B, err_bb);
    etrace_push(code);
    LLVMValueRef rv = errunion_value(F->ret, code, NULL);
    emit_defers(0);
    if (!terminated()) emit_ret(rv);
    LLVMPositionBuilderAtEnd(B, ok_bb);
    if (eu->elem->kind == TY_VOID) return NULL;
    return from_mem(eu->elem, LLVMBuildExtractValue(B, v, 1, ""));
}

static LLVMValueRef gen_catch(Expr *e) {
    Type *eu = ct(e->a->type);
    Type *vt = eu->elem;
    LLVMValueRef v = gen(e->a);
    LLVMValueRef code = LLVMBuildExtractValue(B, v, 0, "");
    LLVMValueRef is_err = LLVMBuildICmp(B, LLVMIntNE, code, LLVMConstInt(T_i16, 0, false), "");
    LLVMBasicBlockRef err_bb = new_bb("catch.err"), ok_bb = new_bb("catch.ok"), merge = new_bb("catch.end");
    LLVMBuildCondBr(B, is_err, err_bb, ok_bb);
    LLVMPositionBuilderAtEnd(B, err_bb);
    etrace_clear();
    if (e->capture_local) store(t_error, code, local_ptr(e->capture_local), false);
    LLVMValueRef hv = NULL;
    LLVMBasicBlockRef hbb = NULL;
    switch (e->hkind) {
    case H_VALUE:
        hv = gen(e->hexpr);
        // a diverging handler (@trap()) leaves the builder in a dead block
        // that is deleted later: it must not feed the phi
        if (!terminated() && (LLVMGetInsertBlock(B) == err_bb || LLVMGetFirstUse(LLVMBasicBlockAsValue(LLVMGetInsertBlock(B))))) {
            hbb = LLVMGetInsertBlock(B);
            LLVMBuildBr(B, merge);
        }
        break;
    case H_RETURN: gen_return(e->hexpr); break;
    case H_BREAK: gen_break(true); break;
    case H_CONTINUE: gen_break(false); break;
    case H_BLOCK:
        gen_block(e->hblock);
        if (!terminated()) LLVMBuildBr(B, merge);  // only possible for !void
        break;
    }
    if (!terminated()) LLVMBuildUnreachable(B);
    LLVMPositionBuilderAtEnd(B, ok_bb);
    LLVMValueRef ov = vt->kind == TY_VOID ? NULL : from_mem(vt, LLVMBuildExtractValue(B, v, 1, ""));
    LLVMBasicBlockRef obb = LLVMGetInsertBlock(B);
    LLVMBuildBr(B, merge);
    LLVMPositionBuilderAtEnd(B, merge);
    if (vt->kind == TY_VOID) return NULL;
    if (!hbb || !hv) return ov;
    LLVMValueRef phi = LLVMBuildPhi(B, vty(vt), "");
    LLVMValueRef vals[2] = {ov, hv};
    LLVMBasicBlockRef bbs[2] = {obb, hbb};
    LLVMAddIncoming(phi, vals, bbs, 2);
    return phi;
}

// ---------------------------------------------------------------------------
// Builtins
// ---------------------------------------------------------------------------
static LLVMValueRef splat(LLVMTypeRef vt, LLVMValueRef x, unsigned lanes) {
    LLVMValueRef v = LLVMBuildInsertElement(B, LLVMGetUndef(vt), x, LLVMConstInt(T_i32, 0, false), "");
    return LLVMBuildShuffleVector(B, v, LLVMGetUndef(vt), LLVMConstNull(LLVMVectorType(T_i32, lanes)), "");
}

// The lanes summed by folding the halves (spec §13.2): lane i plus lane
// i + n/2, until one lane is left.
static LLVMValueRef reduce_add(Type *vt, LLVMValueRef v) {
    bool is_float = vt->elem->kind == TY_FLOAT;
    for (unsigned n = (unsigned)vt->lanes; n > 1; n /= 2) {
        unsigned h = n / 2;
        LLVMValueRef lo[32], hi[32];
        for (unsigned i = 0; i < h; i++) {
            lo[i] = LLVMConstInt(T_i32, i, false);
            hi[i] = LLVMConstInt(T_i32, i + h, false);
        }
        LLVMValueRef a = LLVMBuildShuffleVector(B, v, v, LLVMConstVector(lo, h), "");
        LLVMValueRef b = LLVMBuildShuffleVector(B, v, v, LLVMConstVector(hi, h), "");
        v = is_float ? LLVMBuildFAdd(B, a, b, "") : LLVMBuildAdd(B, a, b, "");
    }
    return LLVMBuildExtractElement(B, v, LLVMConstInt(T_i32, 0, false), "");
}

static LLVMValueRef ext_int(LLVMValueRef v, LLVMTypeRef to, bool is_signed) {
    unsigned a = LLVMGetIntTypeWidth(LLVMTypeOf(v)), b = LLVMGetIntTypeWidth(to);
    if (a == b) return v;
    if (a > b) return LLVMBuildTrunc(B, v, to, "");
    return is_signed ? LLVMBuildSExt(B, v, to, "") : LLVMBuildZExt(B, v, to, "");
}

static LLVMValueRef mem_grow(LLVMValueRef pages);
static LLVMValueRef mem_size(void);
static LLVMValueRef native_rt_fn(const char *name, LLVMTypeRef ret, LLVMTypeRef *ps, int n, LLVMTypeRef *fty);
static LLVMValueRef error_name_table(void);
static void gen_expect_eq(Expr *e);
static LLVMValueRef gen_atomic(Expr *e);

// @swizzle of one 16-byte table (WebAssembly's semantics: an index past
// the table gives 0).
static LLVMValueRef swizzle16(LLVMValueRef x, LLVMValueRef idx) {
    if (g_target.wasm) {
        LLVMValueRef args[2] = {x, idx};
        return intrinsic("llvm.wasm.swizzle", NULL, 0, args, 2);
    }
    if (g_has_ssse3) {
        // pshufb zeroes lanes whose index has its top bit set; adding
        // 0x70 with saturation sets it exactly for indices >= 16
        LLVMTypeRef vt = LLVMTypeOf(idx);
        LLVMValueRef c70[16];
        for (int i = 0; i < 16; i++) c70[i] = LLVMConstInt(T_i8, 0x70, false);
        LLVMValueRef sat[2] = {idx, LLVMConstVector(c70, 16)};
        LLVMValueRef k = intrinsic("llvm.uadd.sat", &vt, 1, sat, 2);
        LLVMValueRef args[2] = {x, k};
        return intrinsic("llvm.x86.ssse3.pshuf.b.128", NULL, 0, args, 2);
    }
    LLVMValueRef r = LLVMConstNull(LLVMTypeOf(x));
    for (unsigned i = 0; i < 16; i++) {
        LLVMValueRef li = LLVMConstInt(T_i32, i, false);
        LLVMValueRef k = LLVMBuildExtractElement(B, idx, li, "");
        LLVMValueRef inb = LLVMBuildICmp(B, LLVMIntULT, k, LLVMConstInt(T_i8, 16, false), "");
        LLVMValueRef kk = LLVMBuildAnd(B, k, LLVMConstInt(T_i8, 15, false), "");
        LLVMValueRef el = LLVMBuildExtractElement(B, x, LLVMBuildZExt(B, kk, T_i32, ""), "");
        el = LLVMBuildSelect(B, inb, el, LLVMConstInt(T_i8, 0, false), "");
        r = LLVMBuildInsertElement(B, r, el, li, "");
    }
    return r;
}

// @int_from_float on vectors (spec §13.2: truncate, saturate, NaN to 0)
// from operations x86 lowers to vector instructions (vcvttps2dq/udq, vmaxps,
// compares and blends); LLVM 18 splits the vector fptosi.sat/fptoui.sat into
// one scalar conversion per lane there (issue 0030). wasm keeps the
// intrinsics, which are its trunc_sat instructions.
static LLVMValueRef vec_int_from_float(Type *rt, Type *ft, LLVMValueRef v) {
    unsigned n = (unsigned)rt->lanes, bits = (unsigned)rt->elem->bits;
    LLVMTypeRef fv = LLVMTypeOf(v), fl = LLVMGetElementType(fv), iv = lty(rt), il = LLVMGetElementType(iv);
    bool f32 = ft->elem->bits == 32;
    // the largest float below 2^e, and 2^e
    double two = ldexp(1.0, rt->elem->is_signed ? (int)bits - 1 : (int)bits);
    double below = f32 ? (double)nextafterf((float)two, 0.0f) : nextafter(two, 0.0);
    LLVMValueRef cb[64], ct2[64], cmn[64], cz[64];
    for (unsigned i = 0; i < n; i++) {
        cb[i] = LLVMConstReal(fl, below);
        ct2[i] = LLVMConstReal(fl, two);
        cmn[i] = LLVMConstReal(fl, -two);
        cz[i] = LLVMConstReal(fl, 0.0);
    }
    LLVMValueRef vb = LLVMConstVector(cb, n), vt = LLVMConstVector(ct2, n), vz = LLVMConstVector(cz, n);
    for (unsigned i = 0; i < n; i++) cz[i] = int_max(il, rt->elem->is_signed);
    LLVMValueRef top = LLVMConstVector(cz, n);
    LLVMValueRef big = LLVMBuildFCmp(B, LLVMRealOGE, v, vt, "");
    if (!rt->elem->is_signed) {
        // ordered compares: NaN and negatives give 0
        LLVMValueRef c = LLVMBuildSelect(B, LLVMBuildFCmp(B, LLVMRealOGT, v, vz, ""), v, vz, "");
        c = LLVMBuildSelect(B, LLVMBuildFCmp(B, LLVMRealOLT, c, vb, ""), c, vb, "");
        return LLVMBuildSelect(B, big, top, LLVMBuildFPToUI(B, c, iv, ""), "");
    }
    LLVMValueRef vm = LLVMConstVector(cmn, n);
    LLVMValueRef c = LLVMBuildSelect(B, LLVMBuildFCmp(B, LLVMRealOGT, v, vm, ""), v, vm, "");
    c = LLVMBuildSelect(B, LLVMBuildFCmp(B, LLVMRealOLT, c, vb, ""), c, vb, "");
    LLVMValueRef r = LLVMBuildSelect(B, big, top, LLVMBuildFPToSI(B, c, iv, ""), "");
    return LLVMBuildSelect(B, LLVMBuildFCmp(B, LLVMRealUNO, v, v, ""), LLVMConstNull(iv), r, "");
}

// Whether every lane of a mask expression is all ones or all zeros: a lane
// comparison, &, |, ^ or ~ of such masks, or an immutable local set to one.
static bool is_mask(Expr *e, int depth) {
    if (depth > 16) return false;
    switch (e->kind) {
    case E_BUILTIN: return e->index >= BI_LANES_EQ && e->index <= BI_LANES_GE;
    case E_BINARY:
        return (e->op == TK_AMP || e->op == TK_PIPE || e->op == TK_CARET) && is_mask(e->a, depth + 1) &&
               is_mask(e->b, depth + 1);
    case E_UNARY: return e->op == TK_TILDE && is_mask(e->a, depth + 1);
    case E_IDENT:
        return e->local && !e->local->mut && F->inits[e->local->index] && is_mask(F->inits[e->local->index], depth + 1);
    default: return false;
    }
}

// Bytes 16k..16k+15 of a byte vector.
static LLVMValueRef piece16(LLVMValueRef v, unsigned k) {
    LLVMValueRef m[16];
    for (unsigned i = 0; i < 16; i++) m[i] = LLVMConstInt(T_i32, 16 * k + i, false);
    return LLVMBuildShuffleVector(B, v, LLVMGetUndef(LLVMTypeOf(v)), LLVMConstVector(m, 16), "");
}

// @swizzle of a byte table of n bytes (n a multiple of 16).
static LLVMValueRef swizzle_bytes(LLVMValueRef x, LLVMValueRef idx, unsigned n) {
    if (n == 16) return swizzle16(x, idx);
    if (n == 64 && g_arch == ARCH_X64 && (g_features & feature_bit("avx512vbmi"))) {
        // vpermb: the index modulo 64, so out-of-range lanes are cleared after
        LLVMValueRef args[2] = {x, idx};
        LLVMValueRef r = intrinsic("llvm.x86.avx512.permvar.qi.512", NULL, 0, args, 2);
        LLVMValueRef c[64];
        for (int i = 0; i < 64; i++) c[i] = LLVMConstInt(T_i8, 64, false);
        LLVMValueRef out = LLVMBuildICmp(B, LLVMIntUGE, idx, LLVMConstVector(c, 64), "");
        return LLVMBuildSelect(B, out, LLVMConstNull(LLVMTypeOf(x)), r, "");
    }
    // one 16-byte lookup per table piece, with the indices moved down by its
    // offset (out of range there: 0), OR-ed; the pieces of the result joined
    unsigned np = n / 16;
    LLVMTypeRef v16 = LLVMVectorType(T_i8, 16);
    LLVMValueRef ps[4];
    for (unsigned k = 0; k < np; k++) {
        LLVMValueRef ix = piece16(idx, k);
        LLVMValueRef acc = LLVMConstNull(v16);
        for (unsigned j = 0; j < np; j++) {
            LLVMValueRef off[16];
            for (int i = 0; i < 16; i++) off[i] = LLVMConstInt(T_i8, 16 * j, false);
            LLVMValueRef ij = LLVMBuildSub(B, ix, LLVMConstVector(off, 16), "");
            acc = LLVMBuildOr(B, acc, swizzle16(piece16(x, j), ij), "");
        }
        ps[k] = acc;
    }
    for (unsigned w = 16; np > 1; np /= 2, w *= 2) {
        LLVMValueRef m[64];
        for (unsigned i = 0; i < 2 * w; i++) m[i] = LLVMConstInt(T_i32, i, false);
        for (unsigned k = 0; k < np / 2; k++) ps[k] = LLVMBuildShuffleVector(B, ps[2 * k], ps[2 * k + 1], LLVMConstVector(m, 2 * w), "");
    }
    return ps[0];
}

static LLVMValueRef const_lanes(LLVMTypeRef lane, unsigned n, unsigned long long v) {
    LLVMValueRef c[64];
    for (unsigned i = 0; i < n; i++) c[i] = LLVMConstInt(lane, v, false);
    return LLVMConstVector(c, n);
}

// x86's one-instruction permutes of a table's lanes by indices of the same
// width (the index modulo the lanes, or the lanes of both tables), where the
// function's extensions have them: the intrinsic's name, or NULL.
static const char *x86_permute(Type *t, bool two) {
    if (g_arch != ARCH_X64) return NULL;
    int w = t->elem->bits, bytes = w * t->lanes / 8;
    bool f = (g_features & feature_bit("avx512f")) != 0;
    if (bytes == 64 && w == 32 && f) return two ? "llvm.x86.avx512.vpermi2var.d.512" : "llvm.x86.avx512.permvar.si.512";
    if (bytes == 64 && w == 64 && f) return two ? "llvm.x86.avx512.vpermi2var.q.512" : "llvm.x86.avx512.permvar.di.512";
    if (bytes == 64 && w == 16 && (g_features & feature_bit("avx512bw")))
        return two ? "llvm.x86.avx512.vpermi2var.hi.512" : "llvm.x86.avx512.permvar.hi.512";
    if (bytes == 64 && w == 8 && (g_features & feature_bit("avx512vbmi")))
        return two ? "llvm.x86.avx512.vpermi2var.qi.512" : "llvm.x86.avx512.permvar.qi.512";
    if (!two && bytes == 32 && w == 32 && (g_features & feature_bit("avx2"))) return "llvm.x86.avx2.permd";
    return NULL;
}

// @swizzle (spec §13.2): lane i of the result is lane idx[i] of the table, or
// 0 when idx[i] (unsigned) is not a lane.
static LLVMValueRef swizzle_lanes(Type *t, LLVMValueRef x, LLVMValueRef idx) {
    unsigned n = (unsigned)t->lanes, w = (unsigned)t->elem->bits, lb = w / 8, nb = n * lb;
    LLVMTypeRef vt = LLVMTypeOf(x), it = LLVMTypeOf(idx);
    const char *perm = x86_permute(t, false);
    if (perm && w > 8) {
        LLVMValueRef args[2] = {LLVMBuildBitCast(B, x, it, ""), idx};
        LLVMValueRef r = intrinsic(perm, NULL, 0, args, 2);
        LLVMValueRef out = LLVMBuildICmp(B, LLVMIntUGE, idx, const_lanes(LLVMGetElementType(it), n, n), "");
        return LLVMBuildBitCast(B, LLVMBuildSelect(B, out, LLVMConstNull(it), r, ""), vt, "");
    }
    LLVMTypeRef bt = LLVMVectorType(T_i8, nb);
    if (w == 8) return LLVMBuildBitCast(B, swizzle_bytes(LLVMBuildBitCast(B, x, bt, ""), idx, nb), vt, "");
    // byte indices: min(idx, n) * lb + k for byte k of each lane (at most 64 + 7)
    LLVMValueRef mn[2] = {idx, const_lanes(LLVMGetElementType(it), n, n)};
    LLVMValueRef cl = intrinsic("llvm.umin", &it, 1, mn, 2);
    cl = LLVMBuildTrunc(B, cl, LLVMVectorType(T_i8, n), "");
    LLVMValueRef rep[64], kk[64];
    for (unsigned i = 0; i < nb; i++) {
        rep[i] = LLVMConstInt(T_i32, i / lb, false);
        kk[i] = LLVMConstInt(T_i8, i % lb, false);
    }
    LLVMValueRef bi = LLVMBuildShuffleVector(B, cl, LLVMGetUndef(LLVMTypeOf(cl)), LLVMConstVector(rep, nb), "");
    bi = LLVMBuildAdd(B, LLVMBuildMul(B, bi, const_lanes(T_i8, nb, lb), ""), LLVMConstVector(kk, nb), "");
    return LLVMBuildBitCast(B, swizzle_bytes(LLVMBuildBitCast(B, x, bt, ""), bi, nb), vt, "");
}

// @swizzle2: the same over the 2n lanes of a, then b.
static LLVMValueRef swizzle2_lanes(Type *t, LLVMValueRef x, LLVMValueRef y, LLVMValueRef idx) {
    unsigned n = (unsigned)t->lanes;
    LLVMTypeRef vt = LLVMTypeOf(x), it = LLVMTypeOf(idx);
    const char *perm = x86_permute(t, true);
    if (perm) {
        LLVMValueRef args[3] = {LLVMBuildBitCast(B, x, it, ""), idx, LLVMBuildBitCast(B, y, it, "")};
        LLVMValueRef r = intrinsic(perm, NULL, 0, args, 3);
        LLVMValueRef out = LLVMBuildICmp(B, LLVMIntUGE, idx, const_lanes(LLVMGetElementType(it), n, 2 * n), "");
        return LLVMBuildBitCast(B, LLVMBuildSelect(B, out, LLVMConstNull(it), r, ""), vt, "");
    }
    // indices below n pick from x (in y they wrap to beyond its lanes), the
    // others from y
    LLVMValueRef lo = swizzle_lanes(t, x, idx);
    LLVMValueRef hi = swizzle_lanes(t, y, LLVMBuildSub(B, idx, const_lanes(LLVMGetElementType(it), n, n), ""));
    LLVMValueRef r = LLVMBuildOr(B, LLVMBuildBitCast(B, lo, it, ""), LLVMBuildBitCast(B, hi, it, ""), "");
    return LLVMBuildBitCast(B, r, vt, "");
}

// The mask of a vector's first min(len, n) lanes.
static LLVMValueRef first_lanes(int n, LLVMValueRef len) {
    LLVMValueRef c[64];
    for (int i = 0; i < n; i++) c[i] = LLVMConstInt(LLVMTypeOf(len), (unsigned long long)i, false);
    LLVMValueRef lv = splat(LLVMVectorType(LLVMTypeOf(len), (unsigned)n), len, (unsigned)n);
    return LLVMBuildICmp(B, LLVMIntULT, LLVMConstVector(c, (unsigned)n), lv, "");
}

// The processor instructions of spec §13.5, as LLVM's intrinsics. A
// 256- or 512-bit AES round or carry-less multiply in a function without
// VAES / VPCLMULQDQ is done on each 128-bit part.
static LLVMValueRef insn_call(const char *name, LLVMTypeRef ret, LLVMValueRef *args, int n) {
    LLVMTypeRef ps[4];
    for (int i = 0; i < n; i++) ps[i] = LLVMTypeOf(args[i]);
    LLVMTypeRef fty = LLVMFunctionType(ret, ps, (unsigned)n, false);
    LLVMValueRef f = LLVMGetNamedFunction(mod, name);
    if (!f) f = LLVMAddFunction(mod, name, fty);
    return LLVMBuildCall2(B, fty, f, args, (unsigned)n, "");
}

static LLVMValueRef v2i64(LLVMValueRef v, unsigned lanes) {
    return LLVMBuildBitCast(B, v, LLVMVectorType(T_i64, lanes), "");
}

static LLVMValueRef part_of(LLVMValueRef v, unsigned k, unsigned n) {
    LLVMValueRef m[8];
    for (unsigned i = 0; i < n; i++) m[i] = LLVMConstInt(T_i32, k * n + i, false);
    return LLVMBuildShuffleVector(B, v, LLVMGetUndef(LLVMTypeOf(v)), LLVMConstVector(m, n), "");
}

static LLVMValueRef gen_insn(Expr *e, Type *rt) {
    int id = e->index;
    int n = e->bargs.len;
    LLVMValueRef a[3];
    for (int i = 0; i < n; i++) a[i] = gen(e->bargs.data[i].e);
    LLVMTypeRef t = lty(rt);
    LLVMTypeRef v4 = LLVMVectorType(T_i32, 4), v2 = LLVMVectorType(T_i64, 2), v16 = LLVMVectorType(T_i8, 16);
    if ((id >= BI_X86_AESENC && id <= BI_X86_AESDECLAST) || id == BI_X86_CLMUL) {
        static const char *const nm[] = {"aesenc", "aesenclast", "aesdec", "aesdeclast"};
        bool clmul = id == BI_X86_CLMUL;
        unsigned bytes = (unsigned)type_size(rt), q = bytes / 8;
        bool wide_ok = bytes == 16 || (g_features & (clmul ? ((uint64_t)1 << 33) : ((uint64_t)1 << 32)));
        if (!wide_ok) {
            // each 128-bit part on its own
            LLVMValueRef r = LLVMGetUndef(LLVMVectorType(T_i64, q));
            for (unsigned k = 0; k < q / 2; k++) {
                LLVMValueRef args[3] = {part_of(v2i64(a[0], q), k, 2), part_of(v2i64(a[1], q), k, 2),
                                        clmul ? LLVMBuildTrunc(B, a[2], T_i8, "") : NULL};
                LLVMValueRef p = insn_call(clmul ? "llvm.x86.pclmulqdq" : arena_printf("llvm.x86.aesni.%s", nm[id - BI_X86_AESENC]),
                                           v2, args, clmul ? 3 : 2);
                for (unsigned j = 0; j < 2; j++)
                    r = LLVMBuildInsertElement(B, r, LLVMBuildExtractElement(B, p, LLVMConstInt(T_i32, j, false), ""),
                                               LLVMConstInt(T_i32, 2 * k + j, false), "");
            }
            return LLVMBuildBitCast(B, r, t, "");
        }
        const char *suffix = bytes == 32 ? ".256" : bytes == 64 ? ".512" : "";
        LLVMValueRef args[3] = {v2i64(a[0], q), v2i64(a[1], q), clmul ? LLVMBuildTrunc(B, a[2], T_i8, "") : NULL};
        const char *name = clmul ? arena_printf("llvm.x86.pclmulqdq%s", suffix)
                                 : arena_printf("llvm.x86.aesni.%s%s", nm[id - BI_X86_AESENC], suffix);
        return LLVMBuildBitCast(B, insn_call(name, LLVMVectorType(T_i64, q), args, clmul ? 3 : 2), t, "");
    }
    switch (id) {
    case BI_X86_AESIMC: {
        LLVMValueRef x = v2i64(a[0], 2);
        return LLVMBuildBitCast(B, insn_call("llvm.x86.aesni.aesimc", v2, &x, 1), t, "");
    }
    case BI_X86_AESKEYGENASSIST: {
        LLVMValueRef args[2] = {v2i64(a[0], 2), LLVMBuildTrunc(B, a[1], T_i8, "")};
        return LLVMBuildBitCast(B, insn_call("llvm.x86.aesni.aeskeygenassist", v2, args, 2), t, "");
    }
    case BI_X86_SHA1RNDS4: {
        LLVMValueRef args[3] = {a[0], a[1], LLVMBuildTrunc(B, a[2], T_i8, "")};
        return insn_call("llvm.x86.sha1rnds4", v4, args, 3);
    }
    case BI_X86_SHA1NEXTE: return insn_call("llvm.x86.sha1nexte", v4, a, 2);
    case BI_X86_SHA1MSG1: return insn_call("llvm.x86.sha1msg1", v4, a, 2);
    case BI_X86_SHA1MSG2: return insn_call("llvm.x86.sha1msg2", v4, a, 2);
    case BI_X86_SHA256RNDS2: return insn_call("llvm.x86.sha256rnds2", v4, a, 3);
    case BI_X86_SHA256MSG1: return insn_call("llvm.x86.sha256msg1", v4, a, 2);
    case BI_X86_SHA256MSG2: return insn_call("llvm.x86.sha256msg2", v4, a, 2);
    case BI_X86_CRC32C: {
        unsigned bits = LLVMGetIntTypeWidth(LLVMTypeOf(a[1]));
        if (bits == 64) {
            LLVMValueRef args[2] = {LLVMBuildZExt(B, a[0], T_i64, ""), a[1]};
            return LLVMBuildTrunc(B, insn_call("llvm.x86.sse42.crc32.64.64", T_i64, args, 2), T_i32, "");
        }
        return insn_call(arena_printf("llvm.x86.sse42.crc32.32.%u", bits), T_i32, a, 2);
    }
    case BI_ARM_AESE: return insn_call("llvm.aarch64.crypto.aese", v16, a, 2);
    case BI_ARM_AESD: return insn_call("llvm.aarch64.crypto.aesd", v16, a, 2);
    case BI_ARM_AESMC: return insn_call("llvm.aarch64.crypto.aesmc", v16, a, 1);
    case BI_ARM_AESIMC: return insn_call("llvm.aarch64.crypto.aesimc", v16, a, 1);
    case BI_ARM_PMULL:
    case BI_ARM_PMULL2: {
        unsigned lane = id == BI_ARM_PMULL2;
        LLVMValueRef args[2] = {LLVMBuildExtractElement(B, a[0], LLVMConstInt(T_i32, lane, false), ""),
                                LLVMBuildExtractElement(B, a[1], LLVMConstInt(T_i32, lane, false), "")};
        return LLVMBuildBitCast(B, insn_call("llvm.aarch64.neon.pmull64", v16, args, 2), t, "");
    }
    case BI_ARM_SHA1C: return insn_call("llvm.aarch64.crypto.sha1c", v4, a, 3);
    case BI_ARM_SHA1P: return insn_call("llvm.aarch64.crypto.sha1p", v4, a, 3);
    case BI_ARM_SHA1M: return insn_call("llvm.aarch64.crypto.sha1m", v4, a, 3);
    case BI_ARM_SHA1H: return insn_call("llvm.aarch64.crypto.sha1h", T_i32, a, 1);
    case BI_ARM_SHA1SU0: return insn_call("llvm.aarch64.crypto.sha1su0", v4, a, 3);
    case BI_ARM_SHA1SU1: return insn_call("llvm.aarch64.crypto.sha1su1", v4, a, 2);
    case BI_ARM_SHA256H: return insn_call("llvm.aarch64.crypto.sha256h", v4, a, 3);
    case BI_ARM_SHA256H2: return insn_call("llvm.aarch64.crypto.sha256h2", v4, a, 3);
    case BI_ARM_SHA256SU0: return insn_call("llvm.aarch64.crypto.sha256su0", v4, a, 2);
    case BI_ARM_SHA256SU1: return insn_call("llvm.aarch64.crypto.sha256su1", v4, a, 3);
    case BI_ARM_SHA512H: return insn_call("llvm.aarch64.crypto.sha512h", v2, a, 3);
    case BI_ARM_SHA512H2: return insn_call("llvm.aarch64.crypto.sha512h2", v2, a, 3);
    case BI_ARM_SHA512SU0: return insn_call("llvm.aarch64.crypto.sha512su0", v2, a, 2);
    case BI_ARM_SHA512SU1: return insn_call("llvm.aarch64.crypto.sha512su1", v2, a, 3);
    case BI_ARM_EOR3:
    case BI_ARM_BCAX: {
        LLVMValueRef args[3] = {LLVMBuildBitCast(B, a[0], v16, ""), LLVMBuildBitCast(B, a[1], v16, ""),
                                LLVMBuildBitCast(B, a[2], v16, "")};
        const char *name = id == BI_ARM_EOR3 ? "llvm.aarch64.crypto.eor3u.v16i8" : "llvm.aarch64.crypto.bcaxu.v16i8";
        return LLVMBuildBitCast(B, insn_call(name, v16, args, 3), t, "");
    }
    case BI_ARM_RAX1: return insn_call("llvm.aarch64.crypto.rax1", v2, a, 2);
    case BI_ARM_XAR: {
        LLVMValueRef args[3] = {a[0], a[1], LLVMBuildZExt(B, a[2], T_i64, "")};
        return insn_call("llvm.aarch64.crypto.xar", v2, args, 3);
    }
    case BI_ARM_CRC32:
    case BI_ARM_CRC32C: {
        unsigned bits = LLVMGetIntTypeWidth(LLVMTypeOf(a[1]));
        const char *sfx = bits == 8 ? "b" : bits == 16 ? "h" : bits == 32 ? "w" : "x";
        LLVMValueRef args[2] = {a[0], bits < 32 ? LLVMBuildZExt(B, a[1], T_i32, "") : a[1]};
        return insn_call(arena_printf("llvm.aarch64.crc32%s%s", id == BI_ARM_CRC32C ? "c" : "", sfx), T_i32, args, 2);
    }
    }
    internal_error("processor instruction builtin");
    return NULL;
}

static LLVMValueRef gen_builtin(Expr *e) {
    BuiltinArg *a = e->bargs.data;
    Type *rt = ct(e->type);
    switch ((BuiltinId)e->index) {
    case BI_SIZE_OF: return usz(type_size(ct(e->tval)));
    case BI_ALIGN_OF: return usz(type_align(ct(e->tval)));
    case BI_OFFSET_OF: return usz(type_field_offset(ct(e->tval), a[1].e->index));
    case BI_MIN_VALUE:
    case BI_MAX_VALUE: {
        i128 lo, hi;
        int_range(rt, &lo, &hi);
        return cint(lty(rt), e->index == BI_MIN_VALUE ? lo : hi);
    }
    case BI_AS: {
        Type *s = ct(a[1].e->type);
        LLVMValueRef v = gen(a[1].e);
        if (s == rt) return v;
        if (s->kind == TY_BOOL) return LLVMBuildZExt(B, v, lty(rt), "");
        if (s->kind == TY_INT && rt->kind == TY_INT) return ext_int(v, lty(rt), s->is_signed);
        if (s->kind == TY_FLOAT && rt->kind == TY_FLOAT) return LLVMBuildFPExt(B, v, lty(rt), "");
        if (s->kind == TY_INT && rt->kind == TY_FLOAT)
            return s->is_signed ? LLVMBuildSIToFP(B, v, lty(rt), "") : LLVMBuildUIToFP(B, v, lty(rt), "");
        return v;
    }
    case BI_INT_CAST: {
        Type *s = ct(a[1].e->type);
        LLVMValueRef v = gen(a[1].e);
        if (s == rt) return v;
        // Range check in the source type's width (no 128-bit arithmetic).
        LLVMTypeRef st = LLVMTypeOf(v);
        int sb = s->bits, tb = rt->bits;
        LLVMValueRef ok = LLVMConstInt(T_i1, 1, false);
        if (s->is_signed) {
            if (!rt->is_signed) ok = LLVMBuildICmp(B, LLVMIntSGE, v, LLVMConstNull(st), "");
            else if (tb < sb) ok = LLVMBuildICmp(B, LLVMIntSGE, v, cint(st, -((i128)1 << (tb - 1))), "");
        }
        int tmax_bits = rt->is_signed ? tb - 1 : tb;  // T.max = 2^tmax_bits - 1
        int smax_bits = s->is_signed ? sb - 1 : sb;
        if (tmax_bits < smax_bits) {
            LLVMValueRef hi = cint(st, ((i128)1 << tmax_bits) - 1);
            LLVMValueRef le = LLVMBuildICmp(B, s->is_signed ? LLVMIntSLE : LLVMIntULE, v, hi, "");
            ok = LLVMBuildAnd(B, ok, le, "");
        }
        check_or_trap(ok, T_INTCAST);
        return ext_int(v, lty(rt), s->is_signed);
    }
    case BI_INT_FROM_ENUM: return gen(a[0].e);
    case BI_ENUM_FROM_INT: {
        // Trap unless the value is one of the enum's members.
        Type *et = e->tval;
        Type *s = ct(a[1].e->type);
        LLVMValueRef v = gen(a[1].e);
        Decl *d = et->decl;
        LLVMBasicBlockRef ok = new_bb("enum.ok");
        LLVMValueRef sw = LLVMBuildSwitch(B, v, trap_block(T_UNIONTAG), (unsigned)d->variants.len);
        i128 lo, hi;
        int_range(s, &lo, &hi);
        for (int i = 0; i < d->variants.len; i++) {
            i128 x = d->variants.data[i].value;
            if (x >= lo && x <= hi) LLVMAddCase(sw, cint(LLVMTypeOf(v), x), ok);
        }
        LLVMPositionBuilderAtEnd(B, ok);
        return ext_int(v, lty(rt), s->is_signed);
    }
    case BI_TRUNCATE: {
        Type *s = ct(a[1].e->type);
        return ext_int(gen(a[1].e), lty(rt), s->is_signed);
    }
    case BI_INT_FROM_FLOAT: {
        LLVMValueRef v = gen(a[1].e);
        if (rt->kind == TY_VEC && !g_target.wasm) return vec_int_from_float(rt, ct(a[1].e->type), v);
        LLVMTypeRef tys[2] = {lty(rt), LLVMTypeOf(v)};
        return intrinsic(rt->is_signed ? "llvm.fptosi.sat" : "llvm.fptoui.sat", tys, 2, &v, 1);
    }
    case BI_FLOAT_FROM_INT: {
        Type *s = ct(a[1].e->type);
        LLVMValueRef v = gen(a[1].e);
        return s->is_signed ? LLVMBuildSIToFP(B, v, lty(rt), "") : LLVMBuildUIToFP(B, v, lty(rt), "");
    }
    case BI_FLOAT_CAST: {
        Type *s = ct(a[1].e->type);
        LLVMValueRef v = gen(a[1].e);
        if (s->bits == rt->bits) return v;
        return s->bits > rt->bits ? LLVMBuildFPTrunc(B, v, lty(rt), "") : LLVMBuildFPExt(B, v, lty(rt), "");
    }
    case BI_BIT_CAST: {
        Type *s = ct(a[1].e->type);
        LLVMValueRef v = gen(a[1].e);
        if (s == rt) return v;
        if (s->kind == TY_PTR && rt->kind != TY_PTR) return LLVMBuildPtrToInt(B, v, lty(rt), "");
        if (rt->kind == TY_PTR && s->kind != TY_PTR) return LLVMBuildIntToPtr(B, v, T_ptr, "");
        if (rt->kind == TY_PTR) return v;
        return LLVMBuildBitCast(B, v, lty(rt), "");
    }
    case BI_INT_FROM_PTR: return LLVMBuildPtrToInt(B, gen(a[0].e), T_isz, "");
    case BI_PTR_FROM_INT: return LLVMBuildIntToPtr(B, gen(a[1].e), T_ptr, "");
    case BI_PTR_CAST: return gen(a[1].e);
    case BI_SLICE_FROM_PTR: {
        LLVMValueRef p = gen(a[0].e), n = gen(a[1].e);
        LLVMValueRef s = LLVMGetUndef(lty(rt));
        s = LLVMBuildInsertValue(B, s, p, 0, "");
        return LLVMBuildInsertValue(B, s, n, 1, "");
    }
    case BI_CAST_SLICE: {
        Type *st = ct(a[1].e->type);
        LLVMValueRef s = gen(a[1].e);
        int64_t from = type_size(st->elem), to = type_size(rt->elem);
        LLVMValueRef len = LLVMBuildExtractValue(B, s, 1, "");
        LLVMValueRef bytes = LLVMBuildMul(B, len, usz(from), "");
        LLVMValueRef nlen;
        if (to == 0) {
            nlen = usz(0);
        } else {
            check_or_trap(LLVMBuildICmp(B, LLVMIntEQ, LLVMBuildURem(B, bytes, usz(to), ""), usz(0), ""), T_LENGTH);
            nlen = LLVMBuildUDiv(B, bytes, usz(to), "");
        }
        return LLVMBuildInsertValue(B, s, nlen, 1, "");
    }
    case BI_MEM_COPY: {
        Type *dt = ct(a[0].e->type);
        LLVMValueRef d = gen(a[0].e), s = gen(a[1].e);
        LLVMValueRef dl = LLVMBuildExtractValue(B, d, 1, ""), sl = LLVMBuildExtractValue(B, s, 1, "");
        check_or_trap(LLVMBuildICmp(B, LLVMIntEQ, dl, sl, ""), T_LENGTH);
        LLVMValueRef bytes = LLVMBuildMul(B, dl, usz(type_size(dt->elem)), "");
        asan_check_slice(d, dt->elem);
        asan_check_slice(s, dt->elem);
        LLVMBuildMemMove(B, LLVMBuildExtractValue(B, d, 0, ""), 1, LLVMBuildExtractValue(B, s, 0, ""), 1, bytes);
        return NULL;
    }
    case BI_MEM_SET: {
        Type *dt = ct(a[0].e->type);
        LLVMValueRef d = gen(a[0].e), v = gen(a[1].e);
        LLVMValueRef ptr = LLVMBuildExtractValue(B, d, 0, ""), len = LLVMBuildExtractValue(B, d, 1, "");
        asan_check_slice(d, dt->elem);
        if (dt->elem->kind == TY_INT && dt->elem->bits == 8) {
            LLVMBuildMemSet(B, ptr, v, len, 1);
            return NULL;
        }
        // generic loop
        LLVMValueRef iv = alloca_in_entry(T_isz, "i");
        LLVMBuildStore(B, usz(0), iv);
        LLVMBasicBlockRef cond = new_bb("memset.cond"), body = new_bb("memset.body"), end = new_bb("memset.end");
        LLVMBuildBr(B, cond);
        LLVMPositionBuilderAtEnd(B, cond);
        LLVMValueRef i = LLVMBuildLoad2(B, T_isz, iv, "");
        LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntULT, i, len, ""), body, end);
        LLVMPositionBuilderAtEnd(B, body);
        store(dt->elem, v, LLVMBuildGEP2(B, lty(dt->elem), ptr, &i, 1, ""), true);
        LLVMBuildStore(B, LLVMBuildAdd(B, i, usz(1), ""), iv);
        LLVMBuildBr(B, cond);
        LLVMPositionBuilderAtEnd(B, end);
        return NULL;
    }
    case BI_MEM_EQ: {
        Type *st = ct(a[0].e->type);
        LLVMValueRef x = gen(a[0].e), y = gen(a[1].e);
        LLVMValueRef xl = LLVMBuildExtractValue(B, x, 1, ""), yl = LLVMBuildExtractValue(B, y, 1, "");
        LLVMValueRef xp = LLVMBuildExtractValue(B, x, 0, ""), yp = LLVMBuildExtractValue(B, y, 0, "");
        LLVMValueRef bytes = LLVMBuildMul(B, xl, usz(type_size(st->elem)), "");
        asan_check_slice(x, st->elem);
        asan_check_slice(y, ct(a[1].e->type)->elem);
        LLVMValueRef res = alloca_in_entry(T_i1, "eq");
        LLVMValueRef iv = alloca_in_entry(T_isz, "i");
        LLVMBuildStore(B, LLVMBuildICmp(B, LLVMIntEQ, xl, yl, ""), res);
        LLVMBuildStore(B, usz(0), iv);
        LLVMBasicBlockRef cond = new_bb("memeq.cond"), body = new_bb("memeq.body"), neq = new_bb("memeq.ne"),
                          end = new_bb("memeq.end");
        LLVMBuildBr(B, cond);
        LLVMPositionBuilderAtEnd(B, cond);
        LLVMValueRef i = LLVMBuildLoad2(B, T_isz, iv, "");
        LLVMValueRef more = LLVMBuildAnd(B, LLVMBuildLoad2(B, T_i1, res, ""), LLVMBuildICmp(B, LLVMIntULT, i, bytes, ""), "");
        LLVMBuildCondBr(B, more, body, end);
        LLVMPositionBuilderAtEnd(B, body);
        LLVMValueRef bx = LLVMBuildLoad2(B, T_i8, LLVMBuildGEP2(B, T_i8, xp, &i, 1, ""), "");
        LLVMValueRef by = LLVMBuildLoad2(B, T_i8, LLVMBuildGEP2(B, T_i8, yp, &i, 1, ""), "");
        LLVMBuildStore(B, LLVMBuildAdd(B, i, usz(1), ""), iv);
        LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntEQ, bx, by, ""), cond, neq);
        LLVMPositionBuilderAtEnd(B, neq);
        LLVMBuildStore(B, LLVMConstInt(T_i1, 0, false), res);
        LLVMBuildBr(B, end);
        LLVMPositionBuilderAtEnd(B, end);
        return LLVMBuildLoad2(B, T_i1, res, "");
    }
    case BI_MEMORY_SIZE: return mem_size();
    case BI_MEMORY_GROW: {
        LLVMValueRef prev = mem_grow(gen(a[0].e));
        LLVMValueRef failed = LLVMBuildICmp(B, LLVMIntEQ, prev, LLVMConstAllOnes(T_isz), "");
        LLVMValueRef code = LLVMBuildSelect(B, failed, errcode_value(error_code_of(intern_cstr("OutOfMemory"))),
                                            LLVMConstInt(T_i16, 0, false), "");
        return errunion_value(rt, code, prev);
    }
    case BI_CLZ:
    case BI_CTZ: {
        LLVMValueRef v = gen(a[0].e);
        LLVMTypeRef t = LLVMTypeOf(v);
        LLVMValueRef args[2] = {v, LLVMConstInt(T_i1, 0, false)};
        return intrinsic(e->index == BI_CLZ ? "llvm.ctlz" : "llvm.cttz", &t, 1, args, 2);
    }
    case BI_POPCOUNT: return intr1("llvm.ctpop", gen(a[0].e));
    case BI_ROTL:
    case BI_ROTR: {
        LLVMValueRef v = gen(a[0].e), k = gen(a[1].e);
        k = ext_int(k, LLVMTypeOf(v), false);
        LLVMTypeRef t = LLVMTypeOf(v);
        LLVMValueRef args[3] = {v, v, k};
        return intrinsic(e->index == BI_ROTL ? "llvm.fshl" : "llvm.fshr", &t, 1, args, 3);
    }
    case BI_ADD_SAT:
    case BI_SUB_SAT: {
        bool sg = rt->kind == TY_INT ? rt->is_signed : rt->elem->is_signed;
        const char *nm = arena_printf("llvm.%c%s.sat", sg ? 's' : 'u', e->index == BI_ADD_SAT ? "add" : "sub");
        return intr2(nm, gen(a[0].e), gen(a[1].e));
    }
    case BI_MUL_SAT: {
        bool saved = O->overflow_saturate;
        O->overflow_saturate = true;
        LLVMValueRef l = gen(a[0].e), r = gen(a[1].e);
        LLVMValueRef v = overflow_op("mul", rt->is_signed, l, r, true);
        O->overflow_saturate = saved;
        return v;
    }
    case BI_ADD_CHECKED:
    case BI_SUB_CHECKED:
    case BI_MUL_CHECKED: {
        Type *t = rt->elem;
        LLVMValueRef l = gen(a[0].e), r = gen(a[1].e);
        const char *op = e->index == BI_ADD_CHECKED ? "add" : e->index == BI_SUB_CHECKED ? "sub" : "mul";
        LLVMValueRef val, ovf;
        op_with_overflow(op, t->is_signed, l, r, &val, &ovf);
        LLVMValueRef code = LLVMBuildSelect(B, ovf, errcode_value(error_code_of(intern_cstr("Overflow"))),
                                            LLVMConstInt(T_i16, 0, false), "");
        return errunion_value(rt, code, val);
    }
    case BI_MIN:
    case BI_MAX: {
        LLVMValueRef l = gen(a[0].e), r = gen(a[1].e);
        bool mn = e->index == BI_MIN;
        if (rt->kind == TY_FLOAT || ty_is_vec_float(rt)) return intr2(mn ? "llvm.minimum" : "llvm.maximum", l, r);
        bool sg = rt->kind == TY_INT ? rt->is_signed : rt->elem->is_signed;
        return intr2(arena_printf("llvm.%c%s", sg ? 's' : 'u', mn ? "min" : "max"), l, r);
    }
    case BI_ABS: {
        LLVMValueRef v = gen(a[0].e);
        if (rt->kind == TY_FLOAT || ty_is_vec_float(rt)) return intr1("llvm.fabs", v);
        LLVMTypeRef t = LLVMTypeOf(v);
        if (rt->kind == TY_INT) {
            LLVMValueRef is_min = LLVMBuildICmp(B, LLVMIntEQ, v, int_min(t), "");
            if (!O->overflow_saturate) check_or_trap(LLVMBuildNot(B, is_min, ""), T_OVERFLOW);
            LLVMValueRef args[2] = {v, LLVMConstInt(T_i1, 0, false)};
            LLVMValueRef r = intrinsic("llvm.abs", &t, 1, args, 2);
            if (O->overflow_saturate) r = LLVMBuildSelect(B, is_min, int_max(t, true), r, "");
            return r;
        }
        LLVMValueRef args[2] = {v, LLVMConstInt(T_i1, 0, false)};
        return intrinsic("llvm.abs", &t, 1, args, 2);  // vectors wrap, as in WASM
    }
    case BI_SQRT: return intr1("llvm.sqrt", gen(a[0].e));
    case BI_FLOOR: return intr1("llvm.floor", gen(a[0].e));
    case BI_CEIL: return intr1("llvm.ceil", gen(a[0].e));
    case BI_TRUNC: return intr1("llvm.trunc", gen(a[0].e));
    case BI_NEAREST: return intr1("llvm.roundeven", gen(a[0].e));
    case BI_FMA: {
        LLVMValueRef x = gen(a[0].e), y = gen(a[1].e), z = gen(a[2].e);
        LLVMTypeRef t = LLVMTypeOf(x);
        LLVMValueRef args[3] = {x, y, z};
        if (g_target.wasm) return rt_call_lanes(is_f32_type(t) ? "fma32" : "fma64", args, 3);
        return intrinsic("llvm.fma", &t, 1, args, 3);
    }
    case BI_REM: {
        LLVMValueRef x = gen(a[0].e), y = gen(a[1].e);
        if (g_target.wasm) {
            LLVMValueRef args[2] = {x, y};
            return rt_call_lanes(is_f32_type(LLVMTypeOf(x)) ? "fmod32" : "fmod64", args, 2);
        }
        return LLVMBuildFRem(B, x, y, "");
    }
    case BI_COPYSIGN: {
        LLVMValueRef x = gen(a[0].e), y = gen(a[1].e);
        return intr2("llvm.copysign", x, y);
    }
    case BI_SPLAT: return splat(lty(rt), gen(a[0].e), (unsigned)rt->lanes);
    case BI_VEC: {
        LLVMValueRef v = LLVMGetUndef(lty(rt));
        for (int i = 1; i < e->bargs.len; i++)
            v = LLVMBuildInsertElement(B, v, gen(a[i].e), LLVMConstInt(T_i32, (unsigned)(i - 1), false), "");
        return v;
    }
    case BI_EXTRACT_LANE: {
        LLVMValueRef v = gen(a[0].e);
        return LLVMBuildExtractElement(B, v, LLVMConstInt(T_i32, (unsigned long long)a[1].e->cval, false), "");
    }
    case BI_REPLACE_LANE: {
        LLVMValueRef v = gen(a[0].e);
        LLVMValueRef x = gen(a[2].e);
        return LLVMBuildInsertElement(B, v, x, LLVMConstInt(T_i32, (unsigned long long)a[1].e->cval, false), "");
    }
    case BI_LOAD: {
        Type *st = ct(a[1].e->type);
        LLVMValueRef s = gen(a[1].e);
        LLVMValueRef len = LLVMBuildExtractValue(B, s, 1, "");
        LLVMValueRef bytes = LLVMBuildMul(B, len, usz(type_size(st->elem)), "");
        int64_t vb = type_size(rt);
        check_or_trap(LLVMBuildICmp(B, LLVMIntUGE, bytes, usz(vb), ""), T_LENGTH);
        asan_check(LLVMBuildExtractValue(B, s, 0, ""), usz(vb));
        return load(rt, LLVMBuildExtractValue(B, s, 0, ""), true);
    }
    case BI_STORE: {
        Type *st = ct(a[0].e->type);
        Type *vt = ct(a[1].e->type);
        LLVMValueRef s = gen(a[0].e);
        LLVMValueRef v = gen(a[1].e);
        LLVMValueRef len = LLVMBuildExtractValue(B, s, 1, "");
        LLVMValueRef bytes = LLVMBuildMul(B, len, usz(type_size(st->elem)), "");
        int64_t vb = type_size(vt);
        check_or_trap(LLVMBuildICmp(B, LLVMIntUGE, bytes, usz(vb), ""), T_LENGTH);
        asan_check(LLVMBuildExtractValue(B, s, 0, ""), usz(vb));
        store(vt, v, LLVMBuildExtractValue(B, s, 0, ""), true);
        return NULL;
    }
    case BI_SHUFFLE: {
        LLVMValueRef x = gen(a[0].e), y = gen(a[1].e);
        Expr *idx = a[2].e;
        LLVMValueRef *m = NEWN(LLVMValueRef, idx->args.len);
        for (int i = 0; i < idx->args.len; i++) m[i] = LLVMConstInt(T_i32, (unsigned long long)idx->args.data[i]->cval, false);
        return LLVMBuildShuffleVector(B, x, y, LLVMConstVector(m, (unsigned)idx->args.len), "");
    }
    case BI_SWIZZLE: return swizzle_lanes(ct(a[0].e->type), gen(a[0].e), gen(a[1].e));
    case BI_SWIZZLE2: return swizzle2_lanes(ct(a[0].e->type), gen(a[0].e), gen(a[1].e), gen(a[2].e));
    case BI_LOAD_PARTIAL: {
        LLVMValueRef s = gen(a[1].e);
        LLVMValueRef p = LLVMBuildExtractValue(B, s, 0, "");
        LLVMValueRef mask = first_lanes(rt->lanes, LLVMBuildExtractValue(B, s, 1, ""));
        LLVMTypeRef tys[2] = {lty(rt), LLVMTypeOf(p)};
        LLVMValueRef args[4] = {p, LLVMConstInt(T_i32, (unsigned long long)type_size(rt->elem), false), mask,
                                LLVMConstNull(lty(rt))};
        return intrinsic("llvm.masked.load", tys, 2, args, 4);
    }
    case BI_STORE_PARTIAL: {
        Type *vt = ct(a[1].e->type);
        LLVMValueRef s = gen(a[0].e);
        LLVMValueRef v = gen(a[1].e);
        LLVMValueRef p = LLVMBuildExtractValue(B, s, 0, "");
        LLVMValueRef mask = first_lanes(vt->lanes, LLVMBuildExtractValue(B, s, 1, ""));
        LLVMTypeRef tys[2] = {lty(vt), LLVMTypeOf(p)};
        LLVMValueRef args[4] = {v, p, LLVMConstInt(T_i32, (unsigned long long)type_size(vt->elem), false), mask};
        intrinsic("llvm.masked.store", tys, 2, args, 4);
        return NULL;
    }
    case BI_LANES_EQ:
    case BI_LANES_NE:
    case BI_LANES_LT:
    case BI_LANES_LE:
    case BI_LANES_GT:
    case BI_LANES_GE: {
        Type *vt = ct(a[0].e->type);
        LLVMValueRef x = gen(a[0].e), y = gen(a[1].e);
        static const int ops[] = {TK_EQEQ, TK_NE, TK_LT, TK_LE, TK_GT, TK_GE};
        LLVMValueRef c = gen_binop(ops[e->index - BI_LANES_EQ], vt, x, y, NULL);
        return LLVMBuildSExt(B, c, lty(rt), "");
    }
    case BI_SELECT: {
        LLVMValueRef m = gen(a[0].e), x = gen(a[1].e), y = gen(a[2].e);
        LLVMTypeRef mt = LLVMTypeOf(m);
        // a mask of lane comparisons: a select on its i1 lanes is the same bit
        // for bit, and becomes a blend (issue 0031)
        Type *mty = ct(a[0].e->type);
        if (rt->kind == TY_VEC && mty->kind == TY_VEC && mty->lanes == rt->lanes && is_mask(a[0].e, 0))
            return LLVMBuildSelect(B, LLVMBuildICmp(B, LLVMIntSLT, m, LLVMConstNull(mt), ""), x, y, "");
        LLVMValueRef xm = LLVMBuildBitCast(B, x, mt, ""), ym = LLVMBuildBitCast(B, y, mt, "");
        LLVMValueRef r = LLVMBuildOr(B, LLVMBuildAnd(B, xm, m, ""), LLVMBuildAnd(B, ym, LLVMBuildNot(B, m, ""), ""), "");
        return LLVMBuildBitCast(B, r, lty(rt), "");
    }
    case BI_ANY_TRUE: {
        LLVMValueRef v = gen(a[0].e);
        LLVMTypeRef it = LLVMIntTypeInContext(ctx, (unsigned)simd_bits(ct(a[0].e->type)));
        LLVMValueRef bits = LLVMBuildBitCast(B, v, it, "");
        return LLVMBuildICmp(B, LLVMIntNE, bits, LLVMConstNull(it), "");
    }
    case BI_ALL_TRUE: {
        LLVMValueRef v = gen(a[0].e);
        Type *vt = ct(a[0].e->type);
        LLVMValueRef iv = vt->elem->kind == TY_FLOAT
                              ? LLVMBuildBitCast(B, v, LLVMVectorType(LLVMIntTypeInContext(ctx, (unsigned)vt->elem->bits),
                                                                      (unsigned)vt->lanes), "")
                              : v;
        LLVMValueRef nz = LLVMBuildICmp(B, LLVMIntNE, iv, LLVMConstNull(LLVMTypeOf(iv)), "");
        return intr1("llvm.vector.reduce.and", nz);
    }
    case BI_BITMASK: {
        Type *vt = ct(a[0].e->type);
        LLVMValueRef v = gen(a[0].e);
        LLVMValueRef neg = LLVMBuildICmp(B, LLVMIntSLT, v, LLVMConstNull(LLVMTypeOf(v)), "");
        LLVMValueRef bits = LLVMBuildBitCast(B, neg, LLVMIntTypeInContext(ctx, (unsigned)vt->lanes), "");
        return vt->lanes >= 32 ? bits : LLVMBuildZExt(B, bits, T_i32, "");  // (u64 for 64 lanes)
    }
    case BI_REDUCE_ADD: return reduce_add(ct(a[0].e->type), gen(a[0].e));
    case BI_DOT: {
        Type *vt = ct(a[0].e->type);
        LLVMValueRef x = gen(a[0].e), y = gen(a[1].e);
        if (vt->elem->kind == TY_FLOAT) return reduce_add(vt, LLVMBuildFMul(B, x, y, ""));
        unsigned nl = (unsigned)vt->lanes;
        LLVMTypeRef w = LLVMVectorType(T_i32, nl);
        LLVMValueRef p = LLVMBuildMul(B, LLVMBuildSExt(B, x, w, ""), LLVMBuildSExt(B, y, w, ""), "");
        LLVMValueRef ev[16], od[16];
        for (unsigned i = 0; i < nl / 2; i++) {
            ev[i] = LLVMConstInt(T_i32, 2 * i, false);
            od[i] = LLVMConstInt(T_i32, 2 * i + 1, false);
        }
        LLVMValueRef pe = LLVMBuildShuffleVector(B, p, LLVMGetUndef(w), LLVMConstVector(ev, nl / 2), "");
        LLVMValueRef po = LLVMBuildShuffleVector(B, p, LLVMGetUndef(w), LLVMConstVector(od, nl / 2), "");
        return LLVMBuildAdd(B, pe, po, "");
    }
    case BI_WIDEN_LOW:
    case BI_WIDEN_HIGH: {
        // WebAssembly's extend_low/extend_high (x86 pmovsx/pmovzx, Arm sxtl/uxtl)
        Type *vt = ct(a[0].e->type);
        LLVMValueRef v = gen(a[0].e);
        unsigned n = (unsigned)rt->lanes, base = e->index == BI_WIDEN_HIGH ? n : 0;
        LLVMValueRef *mask = NEWN(LLVMValueRef, n);
        for (unsigned i = 0; i < n; i++) mask[i] = LLVMConstInt(T_i32, base + i, false);
        LLVMValueRef half = LLVMBuildShuffleVector(B, v, LLVMGetUndef(LLVMTypeOf(v)), LLVMConstVector(mask, n), "");
        return vt->elem->is_signed ? LLVMBuildSExt(B, half, lty(rt), "") : LLVMBuildZExt(B, half, lty(rt), "");
    }
    case BI_NARROW: {
        // WebAssembly's narrow_*_sat, by the operand and result lane types:
        // each lane saturated to the result's range, a's lanes then b's
        Type *vt = ct(a[1].e->type);
        LLVMValueRef x = gen(a[1].e), y = gen(a[2].e);
        unsigned n = (unsigned)vt->lanes, bits = (unsigned)rt->elem->bits;
        LLVMTypeRef xt = LLVMTypeOf(x), lane = LLVMGetElementType(xt);
        i128 hi = rt->elem->is_signed ? ((i128)1 << (bits - 1)) - 1 : ((i128)1 << bits) - 1;
        i128 lo = rt->elem->is_signed ? -((i128)1 << (bits - 1)) : 0;
        if (g_target.wasm && bits <= 16) {
            // narrow_*_sat per pair of 128-bit pieces; they take signed
            // lanes, so unsigned ones are first limited to the result's range
            if (!vt->elem->is_signed) {
                x = intr2("llvm.umin", x, splat(xt, cint(lane, hi), n));
                y = intr2("llvm.umin", y, splat(xt, cint(lane, hi), n));
            }
            unsigned per = 128 / (2 * bits), np = n / per;  // source lanes per piece, pieces per operand
            LLVMTypeRef pt = LLVMVectorType(lane, per), qt = LLVMVectorType(LLVMIntTypeInContext(ctx, bits), 2 * per);
            LLVMValueRef *pieces = NEWN(LLVMValueRef, 2 * np), *mask = NEWN(LLVMValueRef, n * 2);
            for (unsigned k = 0; k < 2 * np; k++) {
                LLVMValueRef src = k < np ? x : y;
                for (unsigned i = 0; i < per; i++) mask[i] = LLVMConstInt(T_i32, (k % np) * per + i, false);
                pieces[k] = np == 1 ? src : LLVMBuildShuffleVector(B, src, LLVMGetUndef(xt), LLVMConstVector(mask, per), "");
            }
            LLVMTypeRef tys[2] = {qt, pt};
            const char *name = rt->elem->is_signed ? "llvm.wasm.narrow.signed" : "llvm.wasm.narrow.unsigned";
            LLVMValueRef *q = NEWN(LLVMValueRef, np);
            for (unsigned k = 0; k < np; k++) {
                LLVMValueRef args[2] = {pieces[2 * k], pieces[2 * k + 1]};
                q[k] = intrinsic(name, tys, 2, args, 2);
            }
            // join the results pairwise (np is 1, 2 or 4)
            for (unsigned m = np, len = 2 * per; m > 1; m /= 2, len *= 2) {
                for (unsigned i = 0; i < 2 * len; i++) mask[i] = LLVMConstInt(T_i32, i, false);
                for (unsigned k = 0; k < m / 2; k++)
                    q[k] = LLVMBuildShuffleVector(B, q[2 * k], q[2 * k + 1], LLVMConstVector(mask, 2 * len), "");
            }
            LLVMValueRef r = q[0];
            return r;
        }
        LLVMValueRef *mask = NEWN(LLVMValueRef, 2 * n);
        for (unsigned i = 0; i < 2 * n; i++) mask[i] = LLVMConstInt(T_i32, i, false);
        LLVMValueRef v = LLVMBuildShuffleVector(B, x, y, LLVMConstVector(mask, 2 * n), "");
        LLVMTypeRef wt = LLVMTypeOf(v);
        if (vt->elem->is_signed) {
            v = intr2("llvm.smax", v, splat(wt, cint(lane, lo), 2 * n));
            v = intr2("llvm.smin", v, splat(wt, cint(lane, hi), 2 * n));
        } else {
            v = intr2("llvm.umin", v, splat(wt, cint(lane, hi), 2 * n));
        }
        return LLVMBuildTrunc(B, v, lty(rt), "");
    }
    case BI_TRAP: emit_trap(T_TRAP); return NULL;
    case BI_MEMORY_ERROR: emit_trap(T_MEMORY); return NULL;
    case BI_SANITIZE_ADDRESS: return LLVMConstInt(T_i1, O->sanitize_address ? 1 : 0, false);
    case BI_TARGET_HAS:
    case BI_TARGET_ARCH:
    case BI_TARGET_OS:
    case BI_CPU_HAS: {
        int tc = target_cond(e, g_features);
        if (tc >= 0) return LLVMConstInt(T_i1, (unsigned long long)tc, false);
        // @cpu_has of an extension the build does not assume
        uint64_t b = feature_bit(e->bargs.data[0].e->str);
        if (!b || g_target.wasm) return LLVMConstInt(T_i1, 0, false);
        return cpu_has_bits(b);
    }
    case BI_X86_CPUID: {
        LLVMValueRef leaf = gen(e->bargs.data[0].e), sub = gen(e->bargs.data[1].e);
        LLVMTypeRef rt[4] = {T_i32, T_i32, T_i32, T_i32};
        LLVMTypeRef st = LLVMStructTypeInContext(ctx, rt, 4, false);
        LLVMTypeRef ps[2] = {T_i32, T_i32};
        LLVMTypeRef fty = LLVMFunctionType(st, ps, 2, false);
        const char *cons = "={ax},={bx},={cx},={dx},{ax},{cx}";
        LLVMValueRef asmv = LLVMGetInlineAsm(fty, "cpuid", 5, (char *)cons, strlen(cons), true, false,
                                             LLVMInlineAsmDialectATT, false);
        LLVMValueRef args[2] = {leaf, sub};
        LLVMValueRef r = LLVMBuildCall2(B, fty, asmv, args, 2, "");
        LLVMValueRef v = LLVMGetUndef(LLVMVectorType(T_i32, 4));
        for (unsigned i = 0; i < 4; i++)
            v = LLVMBuildInsertElement(B, v, LLVMBuildExtractValue(B, r, i, ""), LLVMConstInt(T_i32, i, false), "");
        return v;
    }
    case BI_X86_XGETBV: {
        LLVMValueRef x = gen(e->bargs.data[0].e);
        LLVMTypeRef rt[2] = {T_i32, T_i32};
        LLVMTypeRef st = LLVMStructTypeInContext(ctx, rt, 2, false);
        LLVMTypeRef fty = LLVMFunctionType(st, &T_i32, 1, false);
        const char *cons = "={ax},={dx},{cx}";
        LLVMValueRef asmv = LLVMGetInlineAsm(fty, "xgetbv", 6, (char *)cons, strlen(cons), true, false,
                                             LLVMInlineAsmDialectATT, false);
        LLVMValueRef r = LLVMBuildCall2(B, fty, asmv, &x, 1, "");
        LLVMValueRef lo = LLVMBuildZExt(B, LLVMBuildExtractValue(B, r, 0, ""), T_i64, "");
        LLVMValueRef hi = LLVMBuildZExt(B, LLVMBuildExtractValue(B, r, 1, ""), T_i64, "");
        return LLVMBuildOr(B, lo, LLVMBuildShl(B, hi, LLVMConstInt(T_i64, 32, false), ""), "");
    }
    case BI_EXPECT: check_or_trap(gen(a[0].e), T_EXPECT); return NULL;
    case BI_EXPECT_EQ: gen_expect_eq(e); return NULL;
    case BI_FENCE: LLVMBuildFence(B, LLVMAtomicOrderingSequentiallyConsistent, false, ""); return NULL;
    case BI_ATOMIC_LOAD:
    case BI_ATOMIC_STORE:
    case BI_ATOMIC_ADD:
    case BI_ATOMIC_SUB:
    case BI_ATOMIC_AND:
    case BI_ATOMIC_OR:
    case BI_ATOMIC_XOR:
    case BI_ATOMIC_XCHG:
    case BI_ATOMIC_MIN:
    case BI_ATOMIC_MAX:
    case BI_ATOMIC_CAS:
    case BI_ATOMIC_WAIT:
    case BI_ATOMIC_NOTIFY: return gen_atomic(e);
    case BI_MEMORY_DISCARD: {
        // WebAssembly memory cannot give pages back: nothing to do there
        LLVMValueRef args[2] = {gen(a[0].e), gen(a[1].e)};
        if (!g_target.wasm) {
            LLVMTypeRef fty;
            LLVMTypeRef ps[2] = {T_isz, T_isz};
            LLVMValueRef f = native_rt_fn("ax_memory_discard", T_void, ps, 2, &fty);
            LLVMBuildCall2(B, fty, f, args, 2, "");
        }
        return NULL;
    }
    case BI_ERROR_NAME: {
        LLVMValueRef code = gen(a[0].e);
        LLVMTypeRef fty;
        LLVMValueRef f = error_name_fn(&fty);
        return LLVMBuildCall2(B, fty, f, &code, 1, "");
    }
    default:
        if (e->index >= BI_X86_AESENC && e->index <= BI_ARM_CRC32C) return gen_insn(e, rt);
        break;
    }
    internal_error(arena_printf("builtin @%s not implemented", e->name));
    return NULL;
}

// Atomics (spec §12.8): all sequentially consistent, naturally aligned.
// A misaligned atomic access traps (as WebAssembly requires); on native
// targets the check is explicit so behavior matches.
bool g_needs_native_runtime;

static LLVMValueRef native_rt_fn(const char *name, LLVMTypeRef ret, LLVMTypeRef *ps, int n, LLVMTypeRef *fty) {
    *fty = LLVMFunctionType(ret, ps, (unsigned)n, false);
    LLVMValueRef f = LLVMGetNamedFunction(mod, name);
    if (!f) f = LLVMAddFunction(mod, name, *fty);
    g_needs_native_runtime = true;
    return f;
}

// @expect_eq(a, b): on failure, record both values (and how to show them) in
// axiom.trap_info before trapping. kind: 1 signed, 2 unsigned, 3 float,
// 4 bool, 5 bytes ([]u8), 6 error, 7 address.
static void store_info(int slot, LLVMValueRef v) { LLVMBuildStore(B, v, trap_info_slot(slot)); }

static void gen_expect_eq(Expr *e) {
    Expr *xa = e->bargs.data[0].e, *xb = e->bargs.data[1].e;
    Type *t = ct(xa->type);
    LLVMValueRef a = gen(xa), b = gen(xb);
    LLVMValueRef eq;
    int kind;
    LLVMValueRef ia, ib, la = NULL, lb = NULL;
    switch (t->kind) {
    case TY_FLOAT:
        eq = LLVMBuildFCmp(B, LLVMRealOEQ, a, b, "");
        kind = 3;
        ia = LLVMBuildBitCast(B, t->bits == 32 ? LLVMBuildFPExt(B, a, T_f64, "") : a, T_i64, "");
        ib = LLVMBuildBitCast(B, t->bits == 32 ? LLVMBuildFPExt(B, b, T_f64, "") : b, T_i64, "");
        break;
    case TY_SLICE: {
        LLVMValueRef pa = LLVMBuildExtractValue(B, a, 0, ""), pb = LLVMBuildExtractValue(B, b, 0, "");
        la = LLVMBuildExtractValue(B, a, 1, "");
        lb = LLVMBuildExtractValue(B, b, 1, "");
        // equal lengths and bytes
        LLVMValueRef res = alloca_in_entry(T_i1, "eq");
        LLVMValueRef iv = alloca_in_entry(T_isz, "i");
        LLVMBuildStore(B, LLVMBuildICmp(B, LLVMIntEQ, la, lb, ""), res);
        LLVMBuildStore(B, usz(0), iv);
        LLVMBasicBlockRef cond = new_bb("eq.cond"), body = new_bb("eq.body"), neq = new_bb("eq.ne"), end = new_bb("eq.end");
        LLVMBuildBr(B, cond);
        LLVMPositionBuilderAtEnd(B, cond);
        LLVMValueRef i = LLVMBuildLoad2(B, T_isz, iv, "");
        LLVMValueRef more = LLVMBuildAnd(B, LLVMBuildLoad2(B, T_i1, res, ""), LLVMBuildICmp(B, LLVMIntULT, i, la, ""), "");
        LLVMBuildCondBr(B, more, body, end);
        LLVMPositionBuilderAtEnd(B, body);
        LLVMValueRef bx = LLVMBuildLoad2(B, T_i8, LLVMBuildGEP2(B, T_i8, pa, &i, 1, ""), "");
        LLVMValueRef by = LLVMBuildLoad2(B, T_i8, LLVMBuildGEP2(B, T_i8, pb, &i, 1, ""), "");
        LLVMBuildStore(B, LLVMBuildAdd(B, i, usz(1), ""), iv);
        LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntEQ, bx, by, ""), cond, neq);
        LLVMPositionBuilderAtEnd(B, neq);
        LLVMBuildStore(B, LLVMConstInt(T_i1, 0, false), res);
        LLVMBuildBr(B, end);
        LLVMPositionBuilderAtEnd(B, end);
        eq = LLVMBuildLoad2(B, T_i1, res, "");
        kind = 5;
        ia = LLVMBuildZExt(B, LLVMBuildPtrToInt(B, pa, T_isz, ""), T_i64, "");
        ib = LLVMBuildZExt(B, LLVMBuildPtrToInt(B, pb, T_isz, ""), T_i64, "");
        la = LLVMBuildZExt(B, la, T_i64, "");
        lb = LLVMBuildZExt(B, lb, T_i64, "");
        break;
    }
    case TY_PTR:
    case TY_FN:
        eq = LLVMBuildICmp(B, LLVMIntEQ, a, b, "");
        kind = 7;
        ia = LLVMBuildZExt(B, LLVMBuildPtrToInt(B, a, T_isz, ""), T_i64, "");
        ib = LLVMBuildZExt(B, LLVMBuildPtrToInt(B, b, T_isz, ""), T_i64, "");
        break;
    default: {  // integers, bool, error
        eq = LLVMBuildICmp(B, LLVMIntEQ, a, b, "");
        bool sg = t->kind == TY_INT && t->is_signed;
        kind = t->kind == TY_BOOL ? 4 : t->kind == TY_ERROR ? 6 : sg ? 1 : 2;
        ia = LLVMGetIntTypeWidth(LLVMTypeOf(a)) == 64 ? a : sg ? LLVMBuildSExt(B, a, T_i64, "") : LLVMBuildZExt(B, a, T_i64, "");
        ib = LLVMGetIntTypeWidth(LLVMTypeOf(b)) == 64 ? b : sg ? LLVMBuildSExt(B, b, T_i64, "") : LLVMBuildZExt(B, b, T_i64, "");
        break;
    }
    }
    LLVMBasicBlockRef ok = new_bb("expect.ok"), fail = new_bb("expect.fail");
    LLVMBuildCondBr(B, eq, ok, fail);
    LLVMPositionBuilderAtEnd(B, fail);
    store_info(1, LLVMConstInt(T_i64, (unsigned long long)kind, false));
    store_info(2, ia);
    store_info(3, ib);
    store_info(4, la ? la : LLVMConstInt(T_i64, 0, false));
    store_info(5, lb ? lb : LLVMConstInt(T_i64, 0, false));
    call_trap_site(site_record(T_EXPECT, e->span));
    LLVMBuildUnreachable(B);
    LLVMPositionBuilderAtEnd(B, ok);
}

static LLVMValueRef gen_atomic(Expr *e) {
    BuiltinArg *a = e->bargs.data;
    BuiltinId id = (BuiltinId)e->index;
    Type *pt = ct(a[0].e->type);
    Type *t = pt->elem;
    LLVMValueRef p = gen(a[0].e);
    int64_t size = type_size(t);
    if (size > 1) {
        LLVMValueRef addr = LLVMBuildPtrToInt(B, p, T_isz, "");
        check_or_trap(LLVMBuildICmp(B, LLVMIntEQ, LLVMBuildAnd(B, addr, usz(size - 1), ""), usz(0), ""), T_ATOMIC_ALIGN);
    }
    asan_check(p, usz(size));
    LLVMAtomicOrdering sc = LLVMAtomicOrderingSequentiallyConsistent;
    switch (id) {
    case BI_ATOMIC_LOAD: {
        LLVMValueRef v = LLVMBuildLoad2(B, lty(t), p, "");
        LLVMSetOrdering(v, sc);
        LLVMSetAlignment(v, (unsigned)size);
        return v;
    }
    case BI_ATOMIC_STORE: {
        LLVMValueRef v = gen(a[1].e);
        LLVMValueRef st = LLVMBuildStore(B, v, p);
        LLVMSetOrdering(st, sc);
        LLVMSetAlignment(st, (unsigned)size);
        return NULL;
    }
    case BI_ATOMIC_CAS: {
        LLVMValueRef expv = gen(a[1].e), newv = gen(a[2].e);
        LLVMValueRef r = LLVMBuildAtomicCmpXchg(B, p, expv, newv, sc, sc, false);
        LLVMSetAlignment(r, (unsigned)size);
        return LLVMBuildExtractValue(B, r, 0, "");
    }
    case BI_ATOMIC_WAIT: {
        LLVMValueRef expv = gen(a[1].e), timeout = gen(a[2].e);
        if (g_target.wasm) {
            LLVMValueRef args[3] = {p, expv, timeout};
            return intrinsic("llvm.wasm.memory.atomic.wait32", NULL, 0, args, 3);
        }
        LLVMTypeRef ps[3] = {T_ptr, T_i32, T_i64}, fty;
        LLVMValueRef f = native_rt_fn("ax_wait32", T_i32, ps, 3, &fty);
        LLVMValueRef args[3] = {p, expv, timeout};
        return LLVMBuildCall2(B, fty, f, args, 3, "");
    }
    case BI_ATOMIC_NOTIFY: {
        LLVMValueRef count = gen(a[1].e);
        if (g_target.wasm) {
            LLVMValueRef args[2] = {p, count};
            return intrinsic("llvm.wasm.memory.atomic.notify", NULL, 0, args, 2);
        }
        LLVMTypeRef ps[2] = {T_ptr, T_i32}, fty;
        LLVMValueRef f = native_rt_fn("ax_notify32", T_i32, ps, 2, &fty);
        LLVMValueRef args[2] = {p, count};
        return LLVMBuildCall2(B, fty, f, args, 2, "");
    }
    default: break;
    }
    LLVMAtomicRMWBinOp op;
    bool sg = t->is_signed;
    switch (id) {
    case BI_ATOMIC_ADD: op = LLVMAtomicRMWBinOpAdd; break;
    case BI_ATOMIC_SUB: op = LLVMAtomicRMWBinOpSub; break;
    case BI_ATOMIC_AND: op = LLVMAtomicRMWBinOpAnd; break;
    case BI_ATOMIC_OR: op = LLVMAtomicRMWBinOpOr; break;
    case BI_ATOMIC_XOR: op = LLVMAtomicRMWBinOpXor; break;
    case BI_ATOMIC_XCHG: op = LLVMAtomicRMWBinOpXchg; break;
    case BI_ATOMIC_MIN: op = sg ? LLVMAtomicRMWBinOpMin : LLVMAtomicRMWBinOpUMin; break;
    default: op = sg ? LLVMAtomicRMWBinOpMax : LLVMAtomicRMWBinOpUMax; break;
    }
    LLVMValueRef v = gen(a[1].e);
    LLVMValueRef r = LLVMBuildAtomicRMW(B, op, p, v, sc, false);
    LLVMSetAlignment(r, (unsigned)size);
    return r;
}

// Entry point for threads on wasm: a new thread is a new instance sharing
// the memory; the host calls this export with the thread function, its
// argument and the top of a stack allocated for the thread. The function is
// frameless, so it touches no stack memory before switching stacks.
static void emit_thread_start(void) {
    LLVMTypeRef ps[3] = {T_ptr, T_ptr, T_ptr};
    LLVMTypeRef fty = LLVMFunctionType(T_void, ps, 3, false);
    LLVMValueRef fn = LLVMAddFunction(mod, "axiom_thread_start", fty);
    LLVMAddAttributeAtIndex(fn, LLVMAttributeFunctionIndex,
                            LLVMCreateStringAttribute(ctx, "wasm-export-name", 16, "axiom_thread_start", 18));
    vec_push(g_export_names, intern_cstr("axiom_thread_start"));
    LLVMBasicBlockRef bb = LLVMAppendBasicBlockInContext(ctx, fn, "entry");
    LLVMBasicBlockRef saved = LLVMGetInsertBlock(B);
    LLVMPositionBuilderAtEnd(B, bb);
    LLVMValueRef top = LLVMGetParam(fn, 2);
    // std/thread stores the stack's lowest address at its top
    LLVMBuildStore(B, LLVMBuildLoad2(B, T_isz, top, "bottom"), stack_limit());
    if (stack_traces_on()) {
        // The thread's shadow stack sits at the top of its stack.
        LLVMTypeRef t = ss_type();
        unsigned long long size = (LLVMABISizeOfType(TD, t) + 15) & ~15ull;
        LLVMValueRef neg = LLVMConstInt(T_isz, (unsigned long long)-(long long)size, true);
        LLVMValueRef base = LLVMBuildGEP2(B, T_i8, top, &neg, 1, "");
        LLVMBuildMemSet(B, base, LLVMConstInt(T_i8, 0, false), usz((int64_t)size), 16);
        ss_globals();
        LLVMBuildStore(B, base, ss_global);
        top = base;
    }
    if (tls_globals()) {
        // The thread's thread-locals, below that, from their initial values.
        unsigned long long size = (LLVMABISizeOfType(TD, tls_ty) + 15) & ~15ull;
        LLVMValueRef neg = LLVMConstInt(T_isz, (unsigned long long)-(long long)size, true);
        LLVMValueRef base = LLVMBuildGEP2(B, T_i8, top, &neg, 1, "");
        LLVMBuildMemCpy(B, base, 16, tls_init, 8, usz((int64_t)LLVMABISizeOfType(TD, tls_ty)));
        LLVMBuildStore(B, base, tls_ptr);
        top = base;
    }
    // The thread's stack: set the wasm global __stack_pointer (which the
    // linker defines) directly; llvm.stackrestore in a function without a
    // frame of its own is dropped, and the thread would run on the main
    // thread's stack.
    LLVMBuildStore(B, top, stack_pointer_global());
    LLVMTypeRef ety = LLVMFunctionType(T_void, &T_ptr, 1, false);
    LLVMValueRef arg = LLVMGetParam(fn, 1);
    LLVMBuildCall2(B, ety, LLVMGetParam(fn, 0), &arg, 1, "");
    LLVMBuildRetVoid(B);
    if (saved) LLVMPositionBuilderAtEnd(B, saved);
}

static LLVMValueRef g_errtab;
static LLVMValueRef error_name_table(void) {
    if (g_errtab) return g_errtab;
    int n = g_prog.error_names.len;
    LLVMValueRef *vs = NEWN(LLVMValueRef, n + 1);
    vs[0] = const_slice(string_ptr("<unknown>", 9), 9);
    for (int i = 0; i < n; i++) {
        Sym s = g_prog.error_names.data[i];
        vs[i + 1] = const_slice(string_ptr(s, (uint32_t)strlen(s)), (int64_t)strlen(s));
    }
    LLVMTypeRef st = lty(type_slice(t_u8, false));
    g_errtab = LLVMAddGlobal(mod, LLVMArrayType2(st, (uint64_t)n + 1), "axiom.error_names");
    LLVMSetInitializer(g_errtab, LLVMConstArray2(st, vs, (uint64_t)n + 1));
    LLVMSetLinkage(g_errtab, LLVMPrivateLinkage);
    LLVMSetGlobalConstant(g_errtab, true);
    return g_errtab;
}

// axiom.error_name(code) []u8: the name table lives with the error codes
// (in the link unit when units are compiled separately).
static LLVMValueRef error_name_fn(LLVMTypeRef *fty_out) {
    LLVMTypeRef st = lty(type_slice(t_u8, false));
    LLVMTypeRef fty = LLVMFunctionType(st, &T_i16, 1, false);
    *fty_out = fty;
    LLVMValueRef f = LLVMGetNamedFunction(mod, "axiom.error_name");
    if (f) return f;
    f = LLVMAddFunction(mod, "axiom.error_name", fty);
    if (O->unit == CG_MODULE) return f;
    if (O->unit == CG_WHOLE) LLVMSetLinkage(f, LLVMInternalLinkage);
    LLVMBasicBlockRef saved = LLVMGetInsertBlock(B);
    LLVMMetadataRef loc = dbg_suspend();
    LLVMPositionBuilderAtEnd(B, LLVMAppendBasicBlockInContext(ctx, f, "entry"));
    LLVMValueRef tab = error_name_table();
    int n = g_prog.error_names.len;
    LLVMValueRef c = LLVMBuildZExt(B, LLVMGetParam(f, 0), T_isz, "");
    c = LLVMBuildSelect(B, LLVMBuildICmp(B, LLVMIntULE, c, usz(n), ""), c, usz(0), "");
    LLVMValueRef ix[2] = {usz(0), c};
    LLVMBuildRet(B, LLVMBuildLoad2(B, st, LLVMBuildGEP2(B, LLVMArrayType2(st, (uint64_t)n + 1), tab, ix, 2, ""), ""));
    if (saved) LLVMPositionBuilderAtEnd(B, saved);
    dbg_resume(loc);
    return f;
}

// Linear memory growth. On wasm these are the real instructions; on native
// targets the runtime (ax_memory_size, ax_memory_grow) commits 64 KiB pages of
// one large reserved region, so linear memory is contiguous there too.
static LLVMValueRef mem_size(void) {
    if (g_target.wasm) {
        LLVMValueRef zero = LLVMConstInt(T_i32, 0, false);
        return intrinsic("llvm.wasm.memory.size", &T_isz, 1, &zero, 1);
    }
    LLVMTypeRef fty;
    LLVMValueRef f = native_rt_fn("ax_memory_size", T_isz, NULL, 0, &fty);
    return LLVMBuildCall2(B, fty, f, NULL, 0, "");
}

static LLVMValueRef mem_grow(LLVMValueRef pages) {
    if (g_target.wasm) {
        LLVMValueRef args[2] = {LLVMConstInt(T_i32, 0, false), pages};
        return intrinsic("llvm.wasm.memory.grow", &T_isz, 1, args, 2);
    }
    LLVMTypeRef fty;
    LLVMTypeRef ps[1] = {T_isz};
    LLVMValueRef f = native_rt_fn("ax_memory_grow", T_isz, ps, 1, &fty);
    return LLVMBuildCall2(B, fty, f, &pages, 1, "");
}

// ---------------------------------------------------------------------------
// Expressions
// ---------------------------------------------------------------------------
static LLVMValueRef gen_struct_lit(Expr *e) {
    Type *t = ct(e->type);
    LLVMValueRef v = LLVMGetUndef(lty(t));
    for (int i = 0; i < e->inits.len; i++) {
        Type *ft = type_field(t, i);
        LLVMValueRef fv = gen(e->inits.data[i].value);
        v = LLVMBuildInsertValue(B, v, to_mem(ft, fv), (unsigned)i, "");
    }
    return v;
}

static LLVMValueRef gen_array_lit(Expr *e) {
    Type *t = ct(e->type);
    if (e->kind == E_ARRAY_LIT) {
        LLVMValueRef v = LLVMGetUndef(lty(t));
        for (int i = 0; i < e->args.len; i++)
            v = LLVMBuildInsertValue(B, v, to_mem(t->elem, gen(e->args.data[i])), (unsigned)i, "");
        return v;
    }
    LLVMValueRef x = gen(e->a);
    if (LLVMIsConstant(x) && LLVMIsNull(x)) return LLVMConstNull(lty(t));
    if (t->len <= 16) {
        LLVMValueRef v = LLVMGetUndef(lty(t));
        for (int64_t i = 0; i < t->len; i++) v = LLVMBuildInsertValue(B, v, to_mem(t->elem, x), (unsigned)i, "");
        return v;
    }
    LLVMValueRef tmp = temp_alloca(t);
    LLVMValueRef iv = alloca_in_entry(T_isz, "i");
    LLVMBuildStore(B, usz(0), iv);
    LLVMBasicBlockRef cond = new_bb("fill.cond"), body = new_bb("fill.body"), end = new_bb("fill.end");
    LLVMBuildBr(B, cond);
    LLVMPositionBuilderAtEnd(B, cond);
    LLVMValueRef i = LLVMBuildLoad2(B, T_isz, iv, "");
    LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntULT, i, usz(t->len), ""), body, end);
    LLVMPositionBuilderAtEnd(B, body);
    LLVMValueRef ix[2] = {usz(0), i};
    store(t->elem, x, LLVMBuildGEP2(B, lty(t), tmp, ix, 2, ""), false);
    LLVMBuildStore(B, LLVMBuildAdd(B, i, usz(1), ""), iv);
    LLVMBuildBr(B, cond);
    LLVMPositionBuilderAtEnd(B, end);
    return load(t, tmp, false);
}

static LLVMValueRef gen_unary(Expr *e) {
    Type *t = ct(e->type);
    switch (e->op) {
    case KW_NOT: return LLVMBuildNot(B, gen(e->a), "");
    case TK_AMP: return gen_addr(e->a);
    case TK_TILDE: return LLVMBuildNot(B, gen(e->a), "");
    case TK_MINUS_PCT: return LLVMBuildNeg(B, gen(e->a), "");
    case TK_MINUS: {
        LLVMValueRef v = gen(e->a);
        if (t->kind == TY_FLOAT || ty_is_vec_float(t)) return LLVMBuildFNeg(B, v, "");
        return overflow_op("sub", true, LLVMConstNull(LLVMTypeOf(v)), v, false);
    }
    }
    internal_error("bad unary");
    return NULL;
}

static LLVMValueRef gen_binary(Expr *e) {
    if (e->op == KW_AND || e->op == KW_OR) {
        bool is_and = e->op == KW_AND;
        LLVMValueRef l = gen(e->a);
        LLVMBasicBlockRef lbb = LLVMGetInsertBlock(B);
        LLVMBasicBlockRef rhs = new_bb(is_and ? "and.rhs" : "or.rhs"), end = new_bb(is_and ? "and.end" : "or.end");
        if (is_and) LLVMBuildCondBr(B, l, rhs, end);
        else LLVMBuildCondBr(B, l, end, rhs);
        LLVMPositionBuilderAtEnd(B, rhs);
        LLVMValueRef r = gen(e->b);
        LLVMBasicBlockRef rbb = LLVMGetInsertBlock(B);
        LLVMBuildBr(B, end);
        LLVMPositionBuilderAtEnd(B, end);
        LLVMValueRef phi = LLVMBuildPhi(B, T_i1, "");
        LLVMValueRef vals[2] = {LLVMConstInt(T_i1, is_and ? 0 : 1, false), r};
        LLVMBasicBlockRef bbs[2] = {lbb, rbb};
        LLVMAddIncoming(phi, vals, bbs, 2);
        return phi;
    }
    Type *lt = ct(e->a->type);
    LLVMValueRef l = gen(e->a);
    LLVMValueRef r = gen(e->b);
    return gen_binop(e->op, lt, l, r, ct(e->b->type));
}

static LLVMValueRef gen_slice(Expr *e) {
    Type *at = ct(e->a->type);
    Type *rt = ct(e->type);
    LLVMValueRef ptr, len;
    if (e->op == IDX_SLICE) {
        LLVMValueRef s = gen(e->a);
        ptr = LLVMBuildExtractValue(B, s, 0, "");
        len = LLVMBuildExtractValue(B, s, 1, "");
    } else {
        Type *arr = e->op == IDX_PTR_ARRAY ? at->elem : at;
        ptr = e->op == IDX_PTR_ARRAY ? gen(e->a) : gen_addr(e->a);
        len = array_len_value(arr);
    }
    LLVMValueRef lo = e->b ? gen(e->b) : usz(0);
    LLVMValueRef hi = e->c ? gen(e->c) : len;
    LLVMValueRef ok = LLVMBuildAnd(B, LLVMBuildICmp(B, LLVMIntULE, lo, hi, ""), LLVMBuildICmp(B, LLVMIntULE, hi, len, ""), "");
    check_or_trap(ok, T_BOUNDS);
    LLVMValueRef np = LLVMBuildInBoundsGEP2(B, lty(rt->elem), ptr, &lo, 1, "");
    LLVMValueRef s = LLVMGetUndef(lty(rt));
    s = LLVMBuildInsertValue(B, s, np, 0, "");
    return LLVMBuildInsertValue(B, s, LLVMBuildSub(B, hi, lo, ""), 1, "");
}

static LLVMValueRef gen_inner(Expr *e) {
    Type *t = ct(e->type);
    if (t && t->kind == TY_NEVER) {
        // The expression diverges; generate it and hand back a placeholder.
        if (e->kind == E_BUILTIN) gen_builtin(e);
        Type *want = e->tval ? ct(e->tval) : NULL;
        return want && want->kind != TY_VOID && want->kind != TY_NEVER ? LLVMGetPoison(vty(want)) : NULL;
    }
    if (e->is_const && t) {
        if (t->kind == TY_INT) return cint(lty(t), e->cval);
        if (t->kind == TY_FLOAT) return LLVMConstReal(lty(t), e->fval);
        if (t->kind == TY_BOOL) return LLVMConstInt(T_i1, e->bval ? 1 : 0, false);
    }
    switch (e->kind) {
    case E_IDENT:
        if (e->local) return load(t, local_ptr(e->local), false);
        if (e->tval && e->tval->kind == TY_PARAM && e->tval->is_const) {
            Type *c = ct(e->tval);
            return cint(lty(t), c->cval);
        }
        if (e->decl) {
            switch (e->decl->kind) {
            case D_GLOBAL: return load(t, global_addr(e->decl), false);
            case D_FN:
            case D_HOST_FN: return fn_ref(get_inst(e->decl, subst_gargs(e->gargs, e->ngargs), e->ngargs));
            case D_CONST:
                if (O->unit == CG_MODULE && type_mentions_error(t)) return gen(e->decl->cexpr);
                return gen_const(e->decl->cexpr, t);
            default: break;
            }
        }
        break;
    case E_STRING: return const_slice(string_ptr(e->str, e->slen), e->slen);
    case E_BOOL: return LLVMConstInt(T_i1, e->bval ? 1 : 0, false);
    case E_INT: return cint(lty(t), (i128)e->ival);
    case E_FLOAT: return LLVMConstReal(lty(t), e->fval);
    case E_STRUCT_LIT: return gen_struct_lit(e);
    case E_ARRAY_LIT:
    case E_ARRAY_REPEAT: return gen_array_lit(e);
    case E_ERROR_VAL: return errcode_value((uint32_t)e->cval);
    case E_BUILTIN: return gen_builtin(e);
    case E_UNARY: return gen_unary(e);
    case E_BINARY: return gen_binary(e);
    case E_CATCH: return gen_catch(e);
    case E_TRY: return gen_try(e);
    case E_CALL: return gen_call(e);
    case E_FIELD:
        switch ((FieldKind)e->op) {
        case FK_STRUCT:
            if (is_place(e)) return load(t, asan_place(e, gen_addr(e)), via_pointer(e));
            if (e->self_mode == SELF_DEREF) return load(t, asan_place(e, gen_addr(e)), true);
            return from_mem(t, LLVMBuildExtractValue(B, gen(e->a), (unsigned)e->index, ""));
        case FK_SLICE_LEN: return LLVMBuildExtractValue(B, gen(e->a), 1, "");
        case FK_SLICE_PTR: return LLVMBuildExtractValue(B, gen(e->a), 0, "");
        case FK_ARRAY_LEN: return usz(ct(e->a->type)->len);
        case FK_VARIANT_UNIT: {
            LLVMValueRef tmp = temp_alloca(t);
            LLVMBuildStore(B, LLVMConstInt(lty(union_tag_type(t)), (unsigned long long)e->index, false),
                           LLVMBuildStructGEP2(B, lty(t), tmp, 0, ""));
            return load(t, tmp, false);
        }
        case FK_CONST: return gen_const(e->decl->cexpr, t);
        case FK_DECL:
            if (e->decl->kind == D_GLOBAL) return load(t, global_addr(e->decl), false);
            return fn_ref(get_inst(e->decl, subst_gargs(e->gargs, e->ngargs), e->ngargs));
        }
        break;
    case E_DEREF: return load(t, asan_place(e, gen_addr(e)), true);
    case E_INDEX: return load(t, asan_place(e, gen_addr(e)), via_pointer(e) || e->op != IDX_ARRAY);
    case E_SLICE: return gen_slice(e);
    case E_APPLY: return fn_ref(get_inst(e->decl, subst_gargs(e->gargs, e->ngargs), e->ngargs));
    case E_CONV:
        switch ((ConvKind)e->op) {
        case CONV_NOOP: return gen(e->a);
        case CONV_WRAP_OK: {
            LLVMValueRef v = t->elem->kind == TY_VOID ? NULL : gen(e->a);
            return errunion_value(t, LLVMConstInt(T_i16, 0, false), v);
        }
        case CONV_WRAP_ERR: return errunion_value(t, gen(e->a), NULL);
        }
        break;
    default: break;
    }
    internal_error(arena_printf("cannot generate expression kind %d", e->kind));
    return NULL;
}

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Accumulations that cannot overflow (issue 0029). In a loop with a known
// trip count, `acc += x` with x of a bounded, non-negative range (a narrower
// integer widened with @as, a constant), where acc is a local set to a
// constant just before the loop and changed nowhere else in it, cannot
// overflow when start + trips * the most added per trip fits acc's type: its
// check is left out, so LLVM can unroll and vectorize the loop (summing 25
// u8 into a u32, counting).
// ---------------------------------------------------------------------------
static bool ident_char(char c) { return c == '_' || (c >= '0' && c <= '9') || ((c | 32) >= 'a' && (c | 32) <= 'z'); }

// Whether the source text of sp has the word `name` (names are never
// shadowed, so this over-approximates the uses of a local).
static bool span_mentions(Span sp, Sym name) {
    SourceFile *f = g_files.data[sp.file];
    size_t n = strlen(name);
    for (size_t i = (size_t)sp.start; i + n <= (size_t)sp.end && i + n <= f->len; i++) {
        if (memcmp(f->text + i, name, n) != 0) continue;
        if (i > 0 && ident_char(f->text[i - 1])) continue;
        if (i + n < f->len && ident_char(f->text[i + n])) continue;
        return true;
    }
    return false;
}

// The range of an integer expression, when it is narrower than its type.
static void expr_bounds(Expr *e, Type *t, i128 *lo, i128 *hi) {
    int_range(t, lo, hi);
    if (e->is_const) {
        *lo = *hi = e->cval;
    } else if (e->kind == E_BUILTIN && e->index == BI_AS && e->bargs.len == 2) {
        Type *s = ct(e->bargs.data[1].e->type);
        if (s->kind == TY_INT) int_range(s, lo, hi);
    }
}

// The adds to acc in a loop body, summing their largest values into *hi;
// false if acc is mentioned anywhere else (or in a nested loop), or an add
// can be negative.
static bool loop_adds(Stmt *s, Local *acc, Sym name, i128 *hi, StmtVec *adds) {
    switch (s->kind) {
    case S_BLOCK:
        for (int i = 0; i < s->stmts.len; i++)
            if (!loop_adds(s->stmts.data[i], acc, name, hi, adds)) return false;
        return true;
    case S_ASSIGN:
        if (s->e->kind == E_IDENT && s->e->local == acc && s->op == TK_PLUS_EQ && s->e3 && s->e3->op == TK_PLUS) {
            Type *t = ct(acc->type);
            if (t->kind != TY_INT || span_mentions(s->e2->span, name)) return false;
            i128 l, h;
            expr_bounds(s->e2, t, &l, &h);
            if (l < 0 || h > ((i128)1 << 70)) return false;
            *hi += h;
            vec_push(*adds, s);
            return true;
        }
        return !span_mentions(s->span, name);
    case S_IF:
        if (span_mentions(s->e->span, name) || !loop_adds(s->body, acc, name, hi, adds)) return false;
        return !s->els || loop_adds(s->els, acc, name, hi, adds);
    case S_MATCH:
        if (span_mentions(s->e->span, name)) return false;
        for (int i = 0; i < s->arms.len; i++)
            if (!loop_adds(s->arms.data[i].body, acc, name, hi, adds)) return false;
        return true;
    default: return !span_mentions(s->span, name);
    }
}

static void mark_bounded_sums(Stmt *blk, int at) {
    Stmt *loop = blk->stmts.data[at];
    i128 trips;
    if (loop->e2) {
        if (!loop->e->is_const || !loop->e2->is_const) return;
        trips = loop->e2->cval > loop->e->cval ? loop->e2->cval - loop->e->cval : 0;
    } else {
        Type *it = ct(loop->e->type);
        if (it->kind != TY_ARRAY) return;
        trips = it->len;
    }
    if (trips > ((i128)1 << 40)) return;
    // the accumulators: locals set to a constant by the last statement
    // before the loop that mentions them
    for (int j = at - 1; j >= 0; j--) {
        Stmt *d = blk->stmts.data[j];
        Local *acc = NULL;
        Expr *init = NULL;
        if (d->kind == S_LET && d->local && d->e) {
            acc = d->local;
            init = d->e;
        } else if (d->kind == S_ASSIGN && d->op == TK_EQ && d->e->kind == E_IDENT && d->e->local) {
            acc = d->e->local;
            init = d->e2;
        }
        if (!acc || acc->addr_taken || !init->is_const || ct(acc->type)->kind != TY_INT || init->cval < 0) continue;
        bool later = false;  // mentioned between it and the loop
        for (int k = j + 1; k < at && !later; k++) later = span_mentions(blk->stmts.data[k]->span, acc->name);
        if (later || span_mentions(loop->e->span, acc->name) || (loop->e2 && span_mentions(loop->e2->span, acc->name)))
            continue;
        i128 per = 0;
        StmtVec adds = {0};
        if (!loop_adds(loop->body, acc, acc->name, &per, &adds) || adds.len == 0) continue;
        i128 lo, hi;
        int_range(ct(acc->type), &lo, &hi);
        if (init->cval + trips * per <= hi)
            for (int k = 0; k < adds.len; k++) adds.data[k]->no_overflow = true;
    }
}

static void gen_block(Stmt *b) {
    int height = F->defers.len;
    for (int i = 0; i < b->stmts.len; i++) {
        if (terminated()) break;
        if (b->stmts.data[i]->kind == S_FOR && O->opt_level > 0) mark_bounded_sums(b, i);
        gen_stmt(b->stmts.data[i]);
    }
    if (!terminated()) emit_defers(height);
    F->defers.len = height;
}

static void gen_if(Stmt *s) {
    // A target condition (spec §13.4): only the branch it selects is compiled.
    int tc = target_cond(s->e, g_features);
    if (tc == 1) {
        gen_block(s->body);
        return;
    }
    if (tc == 0) {
        if (s->els) {
            if (s->els->kind == S_IF) gen_if(s->els);
            else gen_block(s->els);
        }
        return;
    }
    LLVMValueRef c = gen(s->e);
    LLVMBasicBlockRef then_bb = new_bb("if.then"), end = new_bb("if.end");
    LLVMBasicBlockRef else_bb = s->els ? new_bb("if.else") : end;
    LLVMBuildCondBr(B, c, then_bb, else_bb);
    LLVMPositionBuilderAtEnd(B, then_bb);
    gen_block(s->body);
    if (!terminated()) LLVMBuildBr(B, end);
    if (s->els) {
        LLVMPositionBuilderAtEnd(B, else_bb);
        if (s->els->kind == S_IF) gen_if(s->els);
        else gen_block(s->els);
        if (!terminated()) LLVMBuildBr(B, end);
    }
    LLVMPositionBuilderAtEnd(B, end);
}

static void push_loop(LLVMBasicBlockRef brk, LLVMBasicBlockRef cont) {
    LoopCtx lc = {brk, cont, F->defers.len};
    vec_push(F->loops, lc);
}

static void gen_for(Stmt *s) {
    Type *vt = ct(s->local->type);
    LLVMValueRef var = local_ptr(s->local);
    LLVMBasicBlockRef cond = new_bb("for.cond"), body = new_bb("for.body"), step = new_bb("for.step"),
                      end = new_bb("for.end");
    if (s->e2) {
        LLVMValueRef lo = gen(s->e);
        LLVMValueRef hi = gen(s->e2);
        LLVMValueRef iv = alloca_in_entry(lty(vt), "for.i");
        LLVMBuildStore(B, lo, iv);
        LLVMBuildBr(B, cond);
        LLVMPositionBuilderAtEnd(B, cond);
        LLVMValueRef i = LLVMBuildLoad2(B, lty(vt), iv, "");
        LLVMBuildCondBr(B, LLVMBuildICmp(B, vt->is_signed ? LLVMIntSLT : LLVMIntULT, i, hi, ""), body, end);
        LLVMPositionBuilderAtEnd(B, body);
        LLVMBuildStore(B, i, var);
        push_loop(end, step);
        gen_block(s->body);
        F->loops.len--;
        if (!terminated()) LLVMBuildBr(B, step);
        LLVMPositionBuilderAtEnd(B, step);
        LLVMValueRef i2 = LLVMBuildLoad2(B, lty(vt), iv, "");
        LLVMBuildStore(B, LLVMBuildAdd(B, i2, LLVMConstInt(lty(vt), 1, false), ""), iv);
        LLVMBuildBr(B, cond);
        LLVMPositionBuilderAtEnd(B, end);
        return;
    }
    Type *ct_ = ct(s->e->type);
    LLVMValueRef ptr, len;
    if (ct_->kind == TY_SLICE) {
        LLVMValueRef sv = gen(s->e);
        ptr = LLVMBuildExtractValue(B, sv, 0, "");
        len = LLVMBuildExtractValue(B, sv, 1, "");
    } else {
        ptr = addr_or_spill(s->e);
        len = usz(ct_->len);
    }
    LLVMValueRef iv = alloca_in_entry(T_isz, "for.idx");
    LLVMBuildStore(B, usz(0), iv);
    LLVMBuildBr(B, cond);
    LLVMPositionBuilderAtEnd(B, cond);
    LLVMValueRef i = LLVMBuildLoad2(B, T_isz, iv, "");
    LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntULT, i, len, ""), body, end);
    LLVMPositionBuilderAtEnd(B, body);
    LLVMValueRef ep = LLVMBuildInBoundsGEP2(B, lty(vt), ptr, &i, 1, "");
    if (ct_->kind == TY_SLICE) asan_check(ep, usz(type_size(vt)));
    store(vt, load(vt, ep, ct_->kind == TY_SLICE), var, false);
    push_loop(end, step);
    gen_block(s->body);
    F->loops.len--;
    if (!terminated()) LLVMBuildBr(B, step);
    LLVMPositionBuilderAtEnd(B, step);
    LLVMValueRef i2 = LLVMBuildLoad2(B, T_isz, iv, "");
    LLVMBuildStore(B, LLVMBuildAdd(B, i2, usz(1), ""), iv);
    LLVMBuildBr(B, cond);
    LLVMPositionBuilderAtEnd(B, end);
}

static void gen_match(Stmt *s) {
    Type *t = ct(s->e->type);
    LLVMBasicBlockRef end = new_bb("match.end");
    int n = s->arms.len;
    LLVMBasicBlockRef *bbs = NEWN(LLVMBasicBlockRef, n + 1);
    int else_idx = -1;
    for (int i = 0; i < n; i++) {
        bbs[i] = new_bb("match.arm");
        if (s->arms.data[i].kind == P_ELSE && else_idx < 0) else_idx = i;
    }
    LLVMValueRef payload_ptr = NULL;
    if (t->kind == TY_UNION) {
        LLVMValueRef up = addr_or_spill(s->e);
        LLVMValueRef tag = LLVMBuildLoad2(B, lty(union_tag_type(t)), LLVMBuildStructGEP2(B, lty(t), up, 0, ""), "");
        if (LLVMCountStructElementTypes(lty(t)) > 1) payload_ptr = LLVMBuildStructGEP2(B, lty(t), up, 1, "");
        int nv = t->decl->variants.len;
        LLVMValueRef sw = LLVMBuildSwitch(B, tag, trap_block(T_UNIONTAG), (unsigned)nv);
        bool *covered = NEWN(bool, nv + 1);
        for (int i = 0; i < n; i++) {
            MatchArm *a = &s->arms.data[i];
            if (a->kind == P_VARIANT && !covered[a->variant]) {
                covered[a->variant] = true;
                LLVMAddCase(sw, LLVMConstInt(lty(union_tag_type(t)), (unsigned long long)a->variant, false), bbs[i]);
            }
        }
        if (else_idx >= 0)
            for (int v = 0; v < nv; v++)
                if (!covered[v]) LLVMAddCase(sw, LLVMConstInt(lty(union_tag_type(t)), (unsigned long long)v, false), bbs[else_idx]);
    } else if (O->unit == CG_MODULE && n > 0 && ct(s->e->type)->kind == TY_ERROR) {
        // Error codes are link-time values here: compare in arm order.
        LLVMValueRef v = gen(s->e);
        for (int i = 0; i < n; i++) {
            MatchArm *a = &s->arms.data[i];
            if (a->kind != P_ERROR) continue;
            LLVMBasicBlockRef next = new_bb("match.next");
            LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntEQ, v, errcode_value(a->error_code), ""), bbs[i], next);
            LLVMPositionBuilderAtEnd(B, next);
        }
        LLVMBuildBr(B, else_idx >= 0 ? bbs[else_idx] : trap_block(T_TRAP));
    } else {
        // integers, bool, error, enum (an enum match without 'else' covers every member)
        LLVMValueRef v = gen(s->e);
        int trap = s->e->type->kind == TY_ENUM ? T_UNIONTAG : T_TRAP;
        LLVMBasicBlockRef dflt = else_idx >= 0 ? bbs[else_idx] : trap_block(trap);
        LLVMValueRef sw = LLVMBuildSwitch(B, v, dflt, (unsigned)n);
        LLVMTypeRef vt_ = LLVMTypeOf(v);
        Vec(i128) seen = {0};
        for (int i = 0; i < n; i++) {
            MatchArm *a = &s->arms.data[i];
            i128 key;
            if (a->kind == P_INT) key = a->ival;
            else if (a->kind == P_BOOL) key = a->bval ? 1 : 0;
            else if (a->kind == P_ERROR) key = a->error_code;
            else if (a->kind == P_VARIANT) key = a->ival;
            else continue;
            bool dup = false;
            for (int k = 0; k < seen.len; k++)
                if (seen.data[k] == key) dup = true;
            if (dup) continue;
            vec_push(seen, key);
            LLVMAddCase(sw, cint(vt_, key), bbs[i]);
        }
        free(seen.data);
    }
    for (int i = 0; i < n; i++) {
        MatchArm *a = &s->arms.data[i];
        LLVMPositionBuilderAtEnd(B, bbs[i]);
        if (a->bind_local && payload_ptr) {
            Type *pt = type_variant(t, a->variant);
            store(pt, load(pt, payload_ptr, true), local_ptr(a->bind_local), false);
        }
        gen_block(a->body);
        if (!terminated()) LLVMBuildBr(B, end);
    }
    LLVMPositionBuilderAtEnd(B, end);
}

static void gen_stmt_inner(Stmt *s) {
    switch (s->kind) {
    case S_LET: {
        LLVMValueRef v = gen(s->e);
        if (terminated() || !v) return;
        store(ct(s->local->type), v, local_ptr(s->local), false);
        if (!s->local->mut) F->inits[s->local->index] = s->e;
        return;
    }
    case S_EXPR: gen(s->e); return;
    case S_DISCARD: gen(s->e); return;
    case S_ASSIGN: {
        Type *t = ct(s->e->type);
        LLVMValueRef addr = gen_addr(s->e);
        bool un = via_pointer(s->e);
        if (s->op == TK_EQ) {
            LLVMValueRef v = gen(s->e2);
            if (!v) return;
            store(t, v, asan_place(s->e, addr), un);
            return;
        }
        asan_place(s->e, addr);
        LLVMValueRef old = load(t, addr, un);
        LLVMValueRef r = gen(s->e2);
        LLVMValueRef nv = s->no_overflow ? (t->is_signed ? LLVMBuildNSWAdd(B, old, r, "") : LLVMBuildNUWAdd(B, old, r, ""))
                                         : gen_binop(s->e3->op, t, old, r, ct(s->e2->type));
        store(t, nv, addr, un);
        return;
    }
    case S_IF: gen_if(s); return;
    case S_WHILE: {
        LLVMBasicBlockRef cond = new_bb("while.cond"), body = new_bb("while.body"), end = new_bb("while.end");
        LLVMBuildBr(B, cond);
        LLVMPositionBuilderAtEnd(B, cond);
        LLVMValueRef c = gen(s->e);
        LLVMBuildCondBr(B, c, body, end);
        LLVMPositionBuilderAtEnd(B, body);
        push_loop(end, cond);
        gen_block(s->body);
        F->loops.len--;
        if (!terminated()) LLVMBuildBr(B, cond);
        LLVMPositionBuilderAtEnd(B, end);
        return;
    }
    case S_LOOP: {
        LLVMBasicBlockRef body = new_bb("loop.body"), end = new_bb("loop.end");
        LLVMBuildBr(B, body);
        LLVMPositionBuilderAtEnd(B, body);
        push_loop(end, body);
        gen_block(s->body);
        F->loops.len--;
        if (!terminated()) LLVMBuildBr(B, body);
        LLVMPositionBuilderAtEnd(B, end);
        return;
    }
    case S_FOR: gen_for(s); return;
    case S_MATCH: gen_match(s); return;
    case S_RETURN: gen_return(s->e); return;
    case S_BREAK: gen_break(true); return;
    case S_CONTINUE: gen_break(false); return;
    case S_DEFER: vec_push(F->defers, s->body); return;
    case S_BLOCK: gen_block(s); return;
    }
}

// Wrappers that track the source span of the code being generated, for
// trap sites.
// (and, with -g, for line tables).
static LLVMValueRef gen(Expr *e) {
    Span saved = g_cur_span;
    g_cur_span = e->span;
    if (DIB) dbg_at(e->span);
    LLVMValueRef v = gen_inner(e);
    g_cur_span = saved;
    if (DIB) dbg_at(saved);
    return v;
}

static LLVMValueRef gen_addr(Expr *e) {
    Span saved = g_cur_span;
    g_cur_span = e->span;
    if (DIB) dbg_at(e->span);
    LLVMValueRef v = gen_addr_inner(e);
    g_cur_span = saved;
    if (DIB) dbg_at(saved);
    return v;
}

static void gen_stmt(Stmt *s) {
    Span saved = g_cur_span;
    g_cur_span = s->span;
    if (DIB) dbg_at(s->span);
    gen_stmt_inner(s);
    g_cur_span = saved;
    if (DIB) dbg_at(saved);
}

// ---------------------------------------------------------------------------
// Function bodies
// ---------------------------------------------------------------------------
// Deletes the blocks that cannot be reached from the entry: continuations
// after return/break/trap, and anything generated in them, loops included.
// Leaving them for the backend makes -O0 code generation several times
// slower. A dead loop's blocks use one another's values, so every use of a
// dead instruction is cut before the blocks go.
static void delete_unreachable(LLVMValueRef fn) {
    int n = (int)LLVMCountBasicBlocks(fn);
    LLVMBasicBlockRef *bbs = NEWN(LLVMBasicBlockRef, n);
    LLVMGetBasicBlocks(fn, bbs);
    // block -> index, by open addressing on the pointer
    int cap = 16;
    while (cap < 2 * n) cap *= 2;
    int *slot = NEWN(int, cap);
    for (int i = 0; i < cap; i++) slot[i] = -1;
    for (int i = 0; i < n; i++) {
        size_t h = ((size_t)(uintptr_t)bbs[i] >> 4) & (size_t)(cap - 1);
        while (slot[h] >= 0) h = (h + 1) & (size_t)(cap - 1);
        slot[h] = i;
    }
    bool *live = NEWN(bool, n);
    int *stack = NEWN(int, n);
    int sp = 0;
    live[0] = true;
    stack[sp++] = 0;
    while (sp > 0) {
        LLVMValueRef term = LLVMGetBasicBlockTerminator(bbs[stack[--sp]]);
        if (!term) continue;
        unsigned ns = LLVMGetNumSuccessors(term);
        for (unsigned k = 0; k < ns; k++) {
            LLVMBasicBlockRef succ = LLVMGetSuccessor(term, k);
            size_t h = ((size_t)(uintptr_t)succ >> 4) & (size_t)(cap - 1);
            while (slot[h] >= 0 && bbs[slot[h]] != succ) h = (h + 1) & (size_t)(cap - 1);
            int j = slot[h];
            if (j >= 0 && !live[j]) {
                live[j] = true;
                stack[sp++] = j;
            }
        }
    }
    for (int i = 0; i < n; i++) {
        if (live[i]) continue;
        for (LLVMValueRef in = LLVMGetFirstInstruction(bbs[i]); in; in = LLVMGetNextInstruction(in)) {
            if (LLVMGetFirstUse(in)) LLVMReplaceAllUsesWith(in, LLVMGetUndef(LLVMTypeOf(in)));
        }
    }
    for (int i = 0; i < n; i++) {
        if (!live[i]) LLVMDeleteBasicBlock(bbs[i]);
    }
}

// The end of a wasm function's entry block (where its allocas are): trap
// with T0008 unless its frame fits above the thread's stack limit.
// Whether the function calls anything but LLVM's intrinsics and the trap
// (so it cannot recurse, and it uses no stack beyond its own frame).
static bool is_leaf(LLVMValueRef fn) {
    for (LLVMBasicBlockRef bb = LLVMGetFirstBasicBlock(fn); bb; bb = LLVMGetNextBasicBlock(bb))
        for (LLVMValueRef i = LLVMGetFirstInstruction(bb); i; i = LLVMGetNextInstruction(i)) {
            if (!LLVMIsACallInst(i) && !LLVMIsAInvokeInst(i)) continue;
            LLVMValueRef callee = LLVMGetCalledValue(i);
            if (!LLVMIsAFunction(callee)) return false;
            size_t n;
            const char *name = LLVMGetValueName2(callee, &n);
            if (strncmp(name, "llvm.", 5) != 0 && strcmp(name, "axiom.trap") != 0) return false;
        }
    return true;
}

// Whether an alloca will become a register (mem2reg): it is only loaded and
// stored whole, as a scalar or vector.
static bool promotable(LLVMValueRef a) {
    LLVMTypeRef t = LLVMGetAllocatedType(a);
    LLVMTypeKind k = LLVMGetTypeKind(t);
    if (k == LLVMStructTypeKind || k == LLVMArrayTypeKind) return false;
    for (LLVMUseRef u = LLVMGetFirstUse(a); u; u = LLVMGetNextUse(u)) {
        LLVMValueRef user = LLVMGetUser(u);
        if (LLVMIsALoadInst(user)) {
            if (LLVMTypeOf(user) != t || LLVMGetVolatile(user)) return false;
        } else if (LLVMIsAStoreInst(user)) {
            if (LLVMGetOperand(user, 0) == a || LLVMTypeOf(LLVMGetOperand(user, 0)) != t || LLVMGetVolatile(user))
                return false;
        } else {
            return false;
        }
    }
    return true;
}

static void check_stack(Decl *d, LLVMBasicBlockRef entry, LLVMBasicBlockRef start) {
    unsigned long long frame = 0;
    // a leaf whose locals all become registers has no frame on the stack
    // (issue 0029: the check was most of the cost of small hot functions)
    // (an inline function cannot recurse either: its calls are its callers')
    bool leaf = d->is_inline || is_leaf(LLVMGetBasicBlockParent(entry));
    for (LLVMValueRef i = LLVMGetFirstInstruction(entry); i; i = LLVMGetNextInstruction(i)) {
        if (!LLVMIsAAllocaInst(i)) continue;
        if (leaf && promotable(i)) continue;
        unsigned long long a = LLVMGetAlignment(i);
        if (a < 1) a = 1;
        frame = (frame + a - 1) / a * a + LLVMABISizeOfType(TD, LLVMGetAllocatedType(i));
    }
    if (frame == 0) {
        LLVMBuildBr(B, start);
        return;
    }
    frame = (frame + 15) & ~15ull;
    LLVMValueRef sp = LLVMBuildPtrToInt(B, LLVMBuildLoad2(B, T_ptr, stack_pointer_global(), "sp"), T_isz, "");
    LLVMValueRef need = LLVMBuildAdd(B, LLVMBuildLoad2(B, T_isz, stack_limit(), "limit"), usz((int64_t)frame), "");
    Span saved = g_cur_span;
    g_cur_span = d->name_span;
    // signed: a stack pointer below address 0 wraps around
    LLVMValueRef room = LLVMBuildSub(B, sp, need, "");
    LLVMBuildCondBr(B, LLVMBuildICmp(B, LLVMIntSLT, room, usz(0), ""), trap_block(T_STACK), start);
    g_cur_span = saved;
}

// Version k of a function with @features or @multiversion: its body
// compiled with extensions `feats`, under the dispatcher's name with a
// suffix, and shown under the function's own name in traces.
static FnInst *get_version(FnInst *fi, int k, uint64_t feats) {
    Sym key = intern_cstr(arena_printf("%s#v%d", fi->key, k));
    FnInst *v = symmap_get(&inst_map, key);
    if (v) return v;
    v = NEW(FnInst);
    *v = *fi;
    v->next = NULL;
    v->key = key;
    v->feats = feats;
    v->dispatch = false;
    const char *suffix = features_string(feats & ~g_features);
    if (!*suffix) suffix = "base";
    size_t nl;
    const char *base = LLVMGetValueName2(fi->fn, &nl);
    v->fn = LLVMAddFunction(mod, arena_printf("%.*s.%s", (int)nl, base, suffix), v->llty);
    set_fn_features(v->fn, feats);
    LLVMSetLinkage(v->fn, LLVMInternalLinkage);
    add_ext_attrs(v->fn, v->fnty, v->sret, false);
    if (pending_tail) pending_tail->next = v;
    else pending_head = v;
    pending_tail = v;
    symmap_put(&inst_map, key, v);
    return v;
}

// std/os/cpu.ax, which every build includes (the start-up check, @cpu_has).
static FnInst *cpu_fn(const char *name) {
    for (int i = 0; i < g_prog.fns.len; i++) {
        Decl *d = g_prog.fns.data[i];
        if (strcmp(d->name, name) || !d->module) continue;
        if (baseline_fn(d)) return get_inst(d, NULL, 0);
    }
    internal_error(arena_printf("std/os/cpu.ax has no %s (is --std right?)", name));
    return NULL;
}

// Does the running processor have every extension in `bits`? (i1)
static LLVMValueRef cpu_has_bits(uint64_t bits) {
    FnInst *fi = cpu_fn("features");
    LLVMValueRef have = LLVMBuildCall2(B, fi->llty, fi->fn, NULL, 0, "");
    LLVMValueRef m = LLVMConstInt(T_i64, bits, false);
    return LLVMBuildICmp(B, LLVMIntEQ, LLVMBuildAnd(B, have, m, ""), m, "");
}

static void dispatch_call(FnInst *fi, FnInst *v, LLVMValueRef *args, unsigned n) {
    LLVMValueRef r = LLVMBuildCall2(B, fi->llty, v->fn, args, n, "");
    add_ext_attrs(r, fi->fnty, fi->sret, true);
    if (LLVMGetTypeKind(LLVMGetReturnType(fi->llty)) == LLVMVoidTypeKind) LLVMBuildRetVoid(B);
    else LLVMBuildRet(B, r);
}

// The body of a function with @features or @multiversion: the first
// version whose extensions the running processor has (the build's own
// count as present), else the @multiversion fallback (the target's
// extensions only), or trap T0013 for @features.
static void gen_dispatch(FnInst *fi) {
    Decl *d = fi->decl;
    LLVMValueRef fn = fi->fn;
    LLVMPositionBuilderAtEnd(B, LLVMAppendBasicBlockInContext(ctx, fn, "entry"));
    g_cur_span = d->attr_span;
    unsigned np = LLVMCountParams(fn);
    LLVMValueRef *args = NEWN(LLVMValueRef, np + 1);
    for (unsigned i = 0; i < np; i++) args[i] = LLVMGetParam(fn, i);
    uint64_t base = fi->feats;
    bool done = false;
    for (int k = 1; k <= d->attr_args.len; k++) {
        uint64_t set = d->feat_sets[k - 1];
        if (set == SEMA_NO_VERSION || done) continue;
        uint64_t want = base | set;
        FnInst *v = get_version(fi, k, want);
        uint64_t missing = want & ~g_features;
        if (!missing) {
            dispatch_call(fi, v, args, np);
            done = true;
            continue;
        }
        LLVMBasicBlockRef yes = LLVMAppendBasicBlockInContext(ctx, fn, "version");
        LLVMBasicBlockRef no = LLVMAppendBasicBlockInContext(ctx, fn, "next");
        LLVMBuildCondBr(B, cpu_has_bits(missing), yes, no);
        LLVMPositionBuilderAtEnd(B, yes);
        dispatch_call(fi, v, args, np);
        LLVMPositionBuilderAtEnd(B, no);
    }
    if (done) return;
    if (d->attr == ATTR_MULTIVERSION) {
        dispatch_call(fi, get_version(fi, 0, base), args, np);
    } else {
        call_trap_site(site_record(T_CPU, d->attr_span));
        LLVMBuildUnreachable(B);
    }
}

static void gen_fn_body_of(FnInst *fi);

static void gen_fn_body(FnInst *fi) {
    if (DIB) LLVMSetCurrentDebugLocation2(B, NULL);
    uint64_t saved_feats = g_features;
    g_features = fi->feats;  // (@target_has, and lowering that depends on extensions)
    if (fi->dispatch) {
        FnState st = {0};
        st.inst = fi;
        st.decl = fi->decl;
        st.fn = fi->fn;
        FnState *saved = F;
        F = &st;
        gen_dispatch(fi);
        F = saved;
    } else {
        gen_fn_body_of(fi);
    }
    g_features = saved_feats;
    if (DIB) LLVMSetCurrentDebugLocation2(B, NULL);
}

static void gen_fn_body_of(FnInst *fi) {
    Decl *d = fi->decl;
    FnState st = {0};
    st.inst = fi;
    st.decl = d;
    st.from = d->all_gparams;
    st.to = fi->gargs;
    st.nsub = fi->ngargs;
    st.fn = fi->fn;
    st.nlocals = d->locals.len;
    st.locals = NEWN(LLVMValueRef, st.nlocals + 1);
    st.inits = NEWN(Expr *, st.nlocals + 1);
    FnState *saved = F;
    F = &st;
    if (DIB && d->span.file >= 0 && d->span.file < g_files.len) {
        int l, c, el, ec;
        span_linecol(d->span, &l, &c, &el, &ec);
        st.di_file = d->span.file;
        LLVMMetadataRef file = di_file(d->span.file);
        LLVMMetadataRef ty = LLVMDIBuilderCreateSubroutineType(DIB, file, NULL, 0, LLVMDIFlagZero);
        size_t ln;
        const char *link = LLVMGetValueName2(fi->fn, &ln);
        st.di_sp = LLVMDIBuilderCreateFunction(DIB, file, fi->display, strlen(fi->display), link, ln, file, (unsigned)l, ty,
                                               LLVMGetLinkage(fi->fn) == LLVMInternalLinkage, true, (unsigned)l,
                                               LLVMDIFlagPrototyped, O->opt_level > 0);
        LLVMSetSubprogram(fi->fn, st.di_sp);
        dbg_at(d->span);
    }
    st.ret = fi->fnty->ret;
    st.entry = LLVMAppendBasicBlockInContext(ctx, fi->fn, "entry");
    LLVMBasicBlockRef start = LLVMAppendBasicBlockInContext(ctx, fi->fn, "start");
    LLVMPositionBuilderAtEnd(B, start);
    int k = 0;
    if (fi->sret) st.sret_ptr = LLVMGetParam(fi->fn, (unsigned)k++);
    for (int i = 0; i < d->params.len; i++) {
        Local *l = d->params.data[i].local;
        Type *pt = fi->fnty->params[i];
        LLVMValueRef p = LLVMGetParam(fi->fn, (unsigned)k++);
        if (by_ptr(pt)) {
            st.locals[l->index] = p;  // params are immutable: use the caller's copy directly
        } else {
            store(pt, p, local_ptr(l), false);
        }
    }
    if (fi->ninl) {
        st.ninl = fi->ninl;
        st.inl = NEWN(LLVMValueRef, fi->ninl);
        for (int i = 0; i < fi->ninl; i++) st.inl[i] = LLVMGetParam(fi->fn, (unsigned)k++);
    }
    gen_block(d->body);
    if (!terminated()) {
        if (st.ret->kind == TY_VOID) LLVMBuildRetVoid(B);
        else if (st.ret->kind == TY_ERRUNION && st.ret->elem->kind == TY_VOID)
            emit_ret(errunion_value(st.ret, LLVMConstInt(T_i16, 0, false), NULL));
        else LLVMBuildUnreachable(B);
    }
    LLVMPositionBuilderAtEnd(B, st.entry);
    if (g_target.wasm) check_stack(d, st.entry, start);
    else LLVMBuildBr(B, start);
    delete_unreachable(fi->fn);
    free(st.defers.data);
    free(st.loops.data);
    F = saved;
}

// ---------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------
static bool setup_target(void) {
    char *err = NULL;
    LLVMTargetRef target;
    const char *triple = g_target.triple;
    if (g_target.wasm) {
        LLVMInitializeWebAssemblyTargetInfo();
        LLVMInitializeWebAssemblyTarget();
        LLVMInitializeWebAssemblyTargetMC();
        LLVMInitializeWebAssemblyAsmPrinter();
    } else {
        LLVMInitializeNativeTarget();
        LLVMInitializeNativeAsmPrinter();
        LLVMInitializeNativeAsmParser();  // (inline asm: @x86_cpuid, @x86_xgetbv)
    }
    if (LLVMGetTargetFromTriple(triple, &target, &err)) {
        fprintf(stderr, "axiom: cannot find LLVM target '%s': %s\n", triple, err);
        LLVMDisposeMessage(err);
        return false;
    }
    // the target's extensions (target.c); functions with @features or
    // @multiversion versions add their own (the target-features attribute)
    // (not multivalue on wasm: LLVM would change how structs are returned)
    const char *features = llvm_features(g_target.wasm ? g_features & ~(uint64_t)0x40 : g_features);
    LLVMCodeGenOptLevel ol = O->opt_level == 0   ? LLVMCodeGenLevelNone
                             : O->opt_level == 1 ? LLVMCodeGenLevelLess
                             : O->opt_level == 2 ? LLVMCodeGenLevelDefault
                                                 : LLVMCodeGenLevelAggressive;
    const char *cpu = g_target.wasm ? "generic" : !strcmp(g_cpu, "native") ? LLVMGetHostCPUName()
                    : g_arch == ARCH_X64 ? "x86-64" : "generic";
    // @swizzle becomes pshufb when the target has SSSE3
    g_has_ssse3 = g_arch == ARCH_X64 && (g_features & 0x2);
    if (g_target.wasm) {
        // LLVM 18's loop strength reduction rewrites element addresses
        // without `inbounds`, after which a constant offset can no longer
        // fold into a load or store (offset=); wasm has no addressing modes
        // for it to improve (issue 0029)
        static bool parsed;
        if (!parsed) {
            const char *argv[] = {"axiom", "-disable-lsr"};
            LLVMParseCommandLineOptions(2, argv, NULL);
            parsed = true;
        }
    }
    TM = LLVMCreateTargetMachine(target, triple, cpu, features, ol, g_target.wasm ? LLVMRelocStatic : LLVMRelocPIC,
                                 LLVMCodeModelDefault);
    TD = LLVMCreateTargetDataLayout(TM);
    LLVMSetTarget(mod, triple);
    LLVMSetModuleDataLayout(mod, TD);
    return true;
}

// Freestanding wasm has no compiler-rt, but LLVM lowers 64-bit multiply
// overflow checks (and other 128-bit multiplies) to the __multi3 libcall.
// Provide it, built from 64-bit operations only and kept out of the
// optimizer so it cannot be turned back into a 128-bit multiply.
static void emit_multi3(void) {
    LLVMTypeRef ps[5] = {T_ptr, T_i64, T_i64, T_i64, T_i64};
    LLVMTypeRef fty = LLVMFunctionType(T_void, ps, 5, false);
    LLVMValueRef fn = LLVMAddFunction(mod, "__multi3", fty);
    LLVMSetLinkage(fn, LLVMWeakAnyLinkage);
    LLVMSetVisibility(fn, LLVMHiddenVisibility);
    const char *attrs[] = {"optnone", "noinline"};
    for (int i = 0; i < 2; i++) {
        unsigned k = LLVMGetEnumAttributeKindForName(attrs[i], strlen(attrs[i]));
        LLVMAddAttributeAtIndex(fn, LLVMAttributeFunctionIndex, LLVMCreateEnumAttribute(ctx, k, 0));
    }
    LLVMBasicBlockRef bb = LLVMAppendBasicBlockInContext(ctx, fn, "entry");
    LLVMPositionBuilderAtEnd(B, bb);
    LLVMValueRef ret = LLVMGetParam(fn, 0), al = LLVMGetParam(fn, 1), ah = LLVMGetParam(fn, 2),
                 bl = LLVMGetParam(fn, 3), bh = LLVMGetParam(fn, 4);
    LLVMValueRef m32 = LLVMConstInt(T_i64, 0xffffffffull, false), s32 = LLVMConstInt(T_i64, 32, false);
    LLVMValueRef a0 = LLVMBuildAnd(B, al, m32, ""), a1 = LLVMBuildLShr(B, al, s32, "");
    LLVMValueRef b0 = LLVMBuildAnd(B, bl, m32, ""), b1 = LLVMBuildLShr(B, bl, s32, "");
    LLVMValueRef p00 = LLVMBuildMul(B, a0, b0, ""), p01 = LLVMBuildMul(B, a0, b1, "");
    LLVMValueRef p10 = LLVMBuildMul(B, a1, b0, ""), p11 = LLVMBuildMul(B, a1, b1, "");
    LLVMValueRef mid = LLVMBuildAdd(B, LLVMBuildAdd(B, LLVMBuildLShr(B, p00, s32, ""), LLVMBuildAnd(B, p01, m32, ""), ""),
                                    LLVMBuildAnd(B, p10, m32, ""), "");
    LLVMValueRef hi = LLVMBuildAdd(B, LLVMBuildAdd(B, p11, LLVMBuildLShr(B, p01, s32, ""), ""),
                                   LLVMBuildAdd(B, LLVMBuildLShr(B, p10, s32, ""), LLVMBuildLShr(B, mid, s32, ""), ""), "");
    LLVMValueRef lo = LLVMBuildOr(B, LLVMBuildAnd(B, p00, m32, ""), LLVMBuildShl(B, mid, s32, ""), "");
    hi = LLVMBuildAdd(B, hi, LLVMBuildAdd(B, LLVMBuildMul(B, al, bh, ""), LLVMBuildMul(B, ah, bl, ""), ""), "");
    LLVMBuildStore(B, lo, ret);
    LLVMValueRef eight = LLVMConstInt(T_i32, 8, false);
    LLVMBuildStore(B, hi, LLVMBuildGEP2(B, T_i8, ret, &eight, 1, ""));
    LLVMBuildRetVoid(B);
}

// A small exported function with a body built by `fill`.
static LLVMValueRef add_export(const char *name, LLVMTypeRef ret, LLVMTypeRef *ps, int n) {
    LLVMTypeRef fty = LLVMFunctionType(ret, ps, (unsigned)n, false);
    LLVMValueRef fn = LLVMAddFunction(mod, name, fty);
    if (g_target.wasm)
        LLVMAddAttributeAtIndex(fn, LLVMAttributeFunctionIndex,
                                LLVMCreateStringAttribute(ctx, "wasm-export-name", 16, name, (unsigned)strlen(name)));
    vec_push(g_export_names, intern_cstr(name));
    LLVMPositionBuilderAtEnd(B, LLVMAppendBasicBlockInContext(ctx, fn, "entry"));
    return fn;
}

// Native programs built for more than the architecture's baseline check the
// processor before main (std/os/cpu.ax's require, from a constructor; both
// are compiled for the baseline).
static void emit_cpu_check(void) {
    FnInst *req = cpu_fn("require");
    LLVMTypeRef fty = LLVMFunctionType(T_void, NULL, 0, false);
    LLVMValueRef fn = LLVMAddFunction(mod, "axiom.cpu_check", fty);
    LLVMSetLinkage(fn, LLVMInternalLinkage);
    set_fn_features(fn, 0);
    LLVMBasicBlockRef cur = LLVMGetInsertBlock(B);
    LLVMPositionBuilderAtEnd(B, LLVMAppendBasicBlockInContext(ctx, fn, "entry"));
    LLVMValueRef args[2] = {LLVMConstInt(T_i64, g_features, false),
                            const_slice(string_ptr(g_cpu, (uint32_t)strlen(g_cpu)), (int64_t)strlen(g_cpu))};
    LLVMBuildCall2(B, req->llty, req->fn, args, 2, "");
    LLVMBuildRetVoid(B);
    if (cur) LLVMPositionBuilderAtEnd(B, cur);
    LLVMTypeRef et[3] = {T_i32, T_ptr, T_ptr};
    LLVMTypeRef ety = LLVMStructTypeInContext(ctx, et, 3, false);
    LLVMValueRef ev[3] = {LLVMConstInt(T_i32, 0, false), fn, LLVMConstNull(T_ptr)};
    LLVMValueRef entry = LLVMConstNamedStruct(ety, ev, 3);
    LLVMValueRef g = LLVMAddGlobal(mod, LLVMArrayType(ety, 1), "llvm.global_ctors");
    LLVMSetLinkage(g, LLVMAppendingLinkage);
    LLVMSetInitializer(g, LLVMConstArray(ety, &entry, 1));
}

// --sanitize=address: `main` is the user's main followed by the leak check
// (mem.asan_exit), which decides the exit status.
static void emit_asan_main(void) {
    Decl *user_main = NULL;
    for (int i = 0; i < g_prog.fns.len; i++) {
        Decl *d = g_prog.fns.data[i];
        if (d->is_export && !strcmp(d->name, "main")) user_main = d;
    }
    if (!user_main) return;
    FnInst *fi = get_inst(user_main, NULL, 0);
    LLVMTypeRef ps[2] = {T_i32, T_ptr};
    LLVMTypeRef fty = LLVMFunctionType(T_i32, ps, 2, false);
    LLVMValueRef fn = LLVMAddFunction(mod, g_target.wasm ? "axiom.export.main" : "main", fty);
    if (g_target.wasm) {
        LLVMAddAttributeAtIndex(fn, LLVMAttributeFunctionIndex,
                                LLVMCreateStringAttribute(ctx, "wasm-export-name", 16, "main", 4));
        vec_push(g_export_names, intern_cstr("axiom.export.main"));
    }
    LLVMPositionBuilderAtEnd(B, LLVMAppendBasicBlockInContext(ctx, fn, "entry"));
    unsigned np = LLVMCountParamTypes(fi->llty);
    LLVMValueRef args[2] = {LLVMGetParam(fn, 0), LLVMGetParam(fn, 1)};
    LLVMValueRef r = LLVMBuildCall2(B, fi->llty, fi->fn, args, np, "");
    LLVMTypeRef rt = LLVMGetReturnType(fi->llty);
    LLVMValueRef code = LLVMConstInt(T_i32, 0, false);
    if (LLVMGetTypeKind(rt) == LLVMIntegerTypeKind) {
        unsigned bits = LLVMGetIntTypeWidth(rt);
        code = bits < 32 ? LLVMBuildZExt(B, r, T_i32, "") : bits > 32 ? LLVMBuildTrunc(B, r, T_i32, "") : r;
    }
    FnInst *ex = mem_fn("asan_exit");
    LLVMValueRef c = LLVMBuildCall2(B, ex->llty, ex->fn, &code, 1, "");
    LLVMBuildRet(B, c);
}

// axiom_shadow_stack(): the calling thread's shadow stack record (see
// ss_ptr), or null when built with --stack-traces=off.
static void emit_shadow_stack_export(void) {
    add_export("axiom_shadow_stack", T_ptr, NULL, 0);
    LLVMBuildRet(B, stack_traces_on() ? ss_ptr() : LLVMConstNull(T_ptr));
}

// axiom_trap_info(): address of {site, kind, a, b, alen, blen} (64-bit
// fields); `site` points at {code, file_id, start, end, line, col, file, text}.
static void emit_trap_info_export(void) {
    add_export("axiom_trap_info", T_ptr, NULL, 0);
    LLVMBuildRet(B, trap_info_global());
}

// Test binaries (spec §17): tests are listed and run through exports.
//   axiom_test_count() u32
//   axiom_test_name(i) *u8        NUL-terminated
//   axiom_test_expect(i) i32      -1 must not trap, -2 any trap, n: trap Tn
//   axiom_test_run(i) u32         0, or the error code the test returned
//   axiom_error_name(code) *u8    NUL-terminated
//   axiom_test_leaks() u32        after a test passed: the allocations it
//                                 never freed, reported on stderr
//                                 (--sanitize=address; otherwise 0)
static void emit_test_exports(void) {
    int n = g_prog.tests.len;
    LLVMValueRef c = add_export("axiom_test_count", T_i32, NULL, 0);
    (void)c;
    LLVMBuildRet(B, LLVMConstInt(T_i32, (unsigned)n, false));

    LLVMValueRef *names = NEWN(LLVMValueRef, n + 1), *expects = NEWN(LLVMValueRef, n + 1);
    for (int i = 0; i < n; i++) {
        Decl *d = g_prog.tests.data[i];
        names[i] = string_ptr(d->test_name, (uint32_t)strlen(d->test_name));
        expects[i] = LLVMConstInt(T_i32, (unsigned long long)(int64_t)d->test_trap, true);
    }
    LLVMValueRef ntab = LLVMAddGlobal(mod, LLVMArrayType2(T_ptr, (uint64_t)n + 1), "axiom.test_names");
    names[n] = LLVMConstNull(T_ptr);
    LLVMSetInitializer(ntab, LLVMConstArray2(T_ptr, names, (uint64_t)n + 1));
    LLVMSetLinkage(ntab, LLVMPrivateLinkage);
    LLVMValueRef etab = LLVMAddGlobal(mod, LLVMArrayType2(T_i32, (uint64_t)n + 1), "axiom.test_expect");
    expects[n] = LLVMConstInt(T_i32, 0, false);
    LLVMSetInitializer(etab, LLVMConstArray2(T_i32, expects, (uint64_t)n + 1));
    LLVMSetLinkage(etab, LLVMPrivateLinkage);

    LLVMTypeRef pi = T_i32;
    LLVMValueRef f = add_export("axiom_test_name", T_ptr, &pi, 1);
    LLVMValueRef ix[2] = {LLVMConstInt(T_i32, 0, false), LLVMGetParam(f, 0)};
    LLVMBuildRet(B, LLVMBuildLoad2(B, T_ptr, LLVMBuildGEP2(B, LLVMArrayType2(T_ptr, (uint64_t)n + 1), ntab, ix, 2, ""), ""));
    f = add_export("axiom_test_expect", T_i32, &pi, 1);
    ix[1] = LLVMGetParam(f, 0);
    LLVMBuildRet(B, LLVMBuildLoad2(B, T_i32, LLVMBuildGEP2(B, LLVMArrayType2(T_i32, (uint64_t)n + 1), etab, ix, 2, ""), ""));

    // axiom_test_run: a switch over the test functions.
    LLVMValueRef run = add_export("axiom_test_run", T_i32, &pi, 1);
    LLVMBasicBlockRef dflt = LLVMAppendBasicBlockInContext(ctx, run, "none");
    LLVMValueRef sw = LLVMBuildSwitch(B, LLVMGetParam(run, 0), dflt, (unsigned)n);
    for (int i = 0; i < n; i++) {
        FnInst *fi = get_inst(g_prog.tests.data[i], NULL, 0);
        LLVMBasicBlockRef bb = LLVMAppendBasicBlockInContext(ctx, run, "test");
        LLVMAddCase(sw, LLVMConstInt(T_i32, (unsigned)i, false), bb);
        LLVMPositionBuilderAtEnd(B, bb);
        LLVMValueRef r = LLVMBuildCall2(B, fi->llty, fi->fn, NULL, 0, "");
        LLVMBuildRet(B, LLVMBuildZExt(B, LLVMBuildExtractValue(B, r, 0, ""), T_i32, ""));
    }
    LLVMPositionBuilderAtEnd(B, dflt);
    LLVMBuildRet(B, LLVMConstInt(T_i32, 0xFFFF, false));

    LLVMValueRef en = add_export("axiom_error_name", T_ptr, &pi, 1);
    LLVMValueRef tab = error_name_table();
    int ne = g_prog.error_names.len;
    LLVMValueRef code = LLVMBuildZExt(B, LLVMGetParam(en, 0), T_isz, "");
    code = LLVMBuildSelect(B, LLVMBuildICmp(B, LLVMIntULE, code, usz(ne), ""), code, usz(0), "");
    LLVMTypeRef st = lty(type_slice(t_u8, false));
    LLVMValueRef ix2[2] = {usz(0), code};
    LLVMValueRef sl = LLVMBuildLoad2(B, st, LLVMBuildGEP2(B, LLVMArrayType2(st, (uint64_t)ne + 1), tab, ix2, 2, ""), "");
    LLVMBuildRet(B, LLVMBuildExtractValue(B, sl, 0, ""));

    add_export("axiom_test_leaks", T_i32, NULL, 0);
    if (O->sanitize_address) {
        FnInst *lk = mem_fn("asan_test_leaks");
        LLVMBuildRet(B, LLVMBuildCall2(B, lk->llty, lk->fn, NULL, 0, ""));
    } else {
        LLVMBuildRet(B, LLVMConstInt(T_i32, 0, false));
    }
}

const char *LLVMGetDefaultTargetTriple_wrapper(void) { return LLVMGetDefaultTargetTriple(); }
const char *LLVMGetHostCPUFeatures_wrapper(void) { return LLVMGetHostCPUFeatures(); }

bool codegen_program(CgOptions *o) {
    O = o;
    ctx = LLVMContextCreate();
    mod = LLVMModuleCreateWithNameInContext(g_files.data[g_prog.root->file->id]->display, ctx);
    B = LLVMCreateBuilderInContext(ctx);
    if (!setup_target()) return false;
    T_i1 = LLVMInt1TypeInContext(ctx);
    T_i8 = LLVMInt8TypeInContext(ctx);
    T_i16 = LLVMInt16TypeInContext(ctx);
    T_i32 = LLVMInt32TypeInContext(ctx);
    T_i64 = LLVMInt64TypeInContext(ctx);
    T_i128 = LLVMIntTypeInContext(ctx, 128);
    T_isz = LLVMIntTypeInContext(ctx, (unsigned)g_target.ptr_bits);
    T_ptr = LLVMPointerTypeInContext(ctx, 0);
    T_void = LLVMVoidTypeInContext(ctx);
    T_f32 = LLVMFloatTypeInContext(ctx);
    T_f64 = LLVMDoubleTypeInContext(ctx);
    T_v128 = LLVMVectorType(T_i32, 4);

    dbg_init();
    if (g_target.wasm) emit_multi3();
    bool program_parts = O->unit != CG_MODULE;
    if (program_parts && g_target.wasm && g_threads) emit_thread_start();

    if (O->unit == CG_WHOLE) {
        // Roots: exported functions. Everything else (functions, generic
        // instances, globals) is generated on first reference, so
        // unreachable code costs nothing in the backend.
        for (int i = 0; i < g_prog.fns.len; i++) {
            Decl *d = g_prog.fns.data[i];
            if (d->is_export) get_inst(d, NULL, 0);
        }
    } else if (O->unit == CG_MODULE) {
        // Every non-generic function of the module (other units may call
        // any of them, directly or from a generic instance).
        for (int i = 0; i < g_prog.fns.len; i++) {
            Decl *d = g_prog.fns.data[i];
            if (d->module == O->module && !d->gparams.len && !(d->owner && d->owner->gparams.len))
                get_inst(d, NULL, 0);
        }
        if (g_test_mode)
            for (int i = 0; i < g_prog.tests.len; i++)
                if (g_prog.tests.data[i]->module == O->module) get_inst(g_prog.tests.data[i], NULL, 0);
    } else {
        // The link unit: globals, error codes and names.
        for (int i = 0; i < g_prog.globals.len; i++) {
            Decl *d = g_prog.globals.data[i];
            if (d->tls && g_target.wasm) tls_globals();
            else global_ref(d);
        }
        for (int i = 0; i < g_prog.error_names.len; i++) {
            LLVMValueRef g = LLVMAddGlobal(mod, T_i16, arena_printf("axiom.errcode.%s", g_prog.error_names.data[i]));
            LLVMSetInitializer(g, LLVMConstInt(T_i16, (unsigned long long)i + 1, false));
            LLVMSetGlobalConstant(g, true);
        }
        LLVMTypeRef fty;
        error_name_fn(&fty);
        if (stack_traces_on()) ss_globals();
        if (g_target.wasm) stack_limit();
        trap_info_global();
    }
    if (program_parts) {
        if (g_test_mode) emit_test_exports();
        if (g_target.wasm) emit_trap_info_export();
        emit_shadow_stack_export();
        if (O->sanitize_address && !g_test_mode) emit_asan_main();
        if (!g_target.wasm && g_features) emit_cpu_check();
    }
    while (pending_head) {
        FnInst *fi = pending_head;
        pending_head = fi->next;
        if (!pending_head) pending_tail = NULL;
        gen_fn_body(fi);
    }
    if (diag_error_count()) return false;
    if (DIB) LLVMDIBuilderFinalize(DIB);

    char *err = NULL;
    if (LLVMVerifyModule(mod, LLVMReturnStatusAction, &err)) {
        fprintf(stderr, "axiom: internal compiler error: invalid LLVM IR generated:\n%s\n", err);
        LLVMDisposeMessage(err);
        if (getenv("AXIOM_DUMP_IR")) LLVMPrintModuleToFile(mod, getenv("AXIOM_DUMP_IR"), NULL);
        return false;
    }
    LLVMDisposeMessage(err);
    // At -O0 a very cheap cleanup (promote locals to SSA, merge trivial
    // blocks) makes the backend faster than skipping it. Inline functions
    // (spec §9.5) are inlined at every level.
    const char *o0_pipeline = getenv("AXIOM_O0_PIPELINE");
    if (!o0_pipeline) o0_pipeline = "always-inline,function(mem2reg,simplifycfg)";
    if (!*o0_pipeline) o0_pipeline = NULL;
    if (O->opt_level > 0 || o0_pipeline) {
        LLVMPassBuilderOptionsRef po = LLVMCreatePassBuilderOptions();
        const char *pipeline = O->opt_level == 0 ? o0_pipeline : O->opt_level == 1 ? "default<O1>" : O->opt_level == 2 ? "default<O2>" : "default<O3>";
        LLVMErrorRef pe = LLVMRunPasses(mod, pipeline, TM, po);
        LLVMDisposePassBuilderOptions(po);
        if (pe) {
            char *m = LLVMGetErrorMessage(pe);
            fprintf(stderr, "axiom: optimization failed: %s\n", m);
            LLVMDisposeErrorMessage(m);
            return false;
        }
    }
    if (O->emit == EMIT_LLVM) {
        if (LLVMPrintModuleToFile(mod, O->out_path, &err)) {
            fprintf(stderr, "axiom: cannot write %s: %s\n", O->out_path, err);
            return false;
        }
        return true;
    }
    if (LLVMTargetMachineEmitToFile(TM, mod, (char *)O->out_path, O->emit == EMIT_ASM ? LLVMAssemblyFile : LLVMObjectFile,
                                    &err)) {
        fprintf(stderr, "axiom: code generation failed: %s\n", err);
        LLVMDisposeMessage(err);
        return false;
    }
    return true;
}

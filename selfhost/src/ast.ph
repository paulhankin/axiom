// =============================================================================
// ast.ph - the abstract syntax tree and semantic types. Sema annotates the
// tree in place. (Types live here too because types and declarations refer
// to each other, and modules cannot import each other cyclically.)
// =============================================================================
import "package.ph" as pkg;
import "util.ph" as u;

pub const TE_PRIM: i32 = 0;
pub const TE_NAMED: i32 = 1;
pub const TE_PTR: i32 = 2;
pub const TE_SLICE: i32 = 3;
pub const TE_ARRAY: i32 = 4;
pub const TE_FN: i32 = 5;
pub const TE_ANON_STRUCT: i32 = 6;
pub const TE_ERRUNION: i32 = 7;
pub const TE_ERROR: i32 = 8;
pub const TE_SELF: i32 = 9;
pub const TE_INTLIT: i32 = 10;

pub const E_IDENT: i32 = 0;
pub const E_SELF: i32 = 1;
pub const E_INT: i32 = 2;
pub const E_FLOAT: i32 = 3;
pub const E_STRING: i32 = 4;
pub const E_BOOL: i32 = 5;
pub const E_STRUCT_LIT: i32 = 6;
pub const E_ARRAY_LIT: i32 = 7;
pub const E_ARRAY_REPEAT: i32 = 8;
pub const E_ERROR_VAL: i32 = 9;
pub const E_BUILTIN: i32 = 10;
pub const E_UNARY: i32 = 12;
pub const E_BINARY: i32 = 13;
pub const E_CATCH: i32 = 14;
pub const E_FIELD: i32 = 15;
pub const E_DEREF: i32 = 16;
pub const E_CALL: i32 = 17;
pub const E_INDEX: i32 = 18;
pub const E_SLICE: i32 = 19;
pub const E_TRY: i32 = 20;
pub const E_APPLY: i32 = 21;
pub const E_CONV: i32 = 22;

pub const CONV_NOOP: i32 = 0;
pub const CONV_WRAP_OK: i32 = 1;
pub const CONV_WRAP_ERR: i32 = 2;

pub const CALLK_DIRECT: i32 = 0;
pub const CALLK_INDIRECT: i32 = 1;
pub const CALLK_VARIANT: i32 = 2;

pub const FK_STRUCT: i32 = 0;
pub const FK_SLICE_LEN: i32 = 1;
pub const FK_SLICE_PTR: i32 = 2;
pub const FK_ARRAY_LEN: i32 = 3;
pub const FK_VARIANT_UNIT: i32 = 4;
pub const FK_CONST: i32 = 5;
pub const FK_DECL: i32 = 6;

pub const CAT_VALUE: i32 = 0;
pub const CAT_TYPE: i32 = 1;
pub const CAT_MODULE: i32 = 2;
pub const CAT_FUNC: i32 = 3;
pub const CAT_METHOD: i32 = 4;
pub const CAT_VARIANT: i32 = 5;
pub const CAT_GENERIC: i32 = 6;

pub const H_VALUE: i32 = 0;
pub const H_RETURN: i32 = 1;
pub const H_BREAK: i32 = 2;
pub const H_CONTINUE: i32 = 3;
pub const H_BLOCK: i32 = 4;

pub const SELF_NONE: i32 = 0;
pub const SELF_VALUE: i32 = 1;
pub const SELF_ADDR: i32 = 2;
pub const SELF_PASS: i32 = 3;
pub const SELF_DEREF: i32 = 4;

pub const S_LET: i32 = 0;
pub const S_EXPR: i32 = 1;
pub const S_ASSIGN: i32 = 2;
pub const S_DISCARD: i32 = 3;
pub const S_IF: i32 = 4;
pub const S_WHILE: i32 = 5;
pub const S_LOOP: i32 = 6;
pub const S_FOR: i32 = 7;
pub const S_MATCH: i32 = 8;
pub const S_RETURN: i32 = 9;
pub const S_BREAK: i32 = 10;
pub const S_CONTINUE: i32 = 11;
pub const S_DEFER: i32 = 12;
pub const S_BLOCK: i32 = 14;

pub const P_VARIANT: i32 = 0;
pub const P_ERROR: i32 = 1;
pub const P_INT: i32 = 2;
pub const P_BOOL: i32 = 3;
pub const P_ELSE: i32 = 4;

pub const D_IMPORT: i32 = 0;
pub const D_HOST_FN: i32 = 1;
pub const D_ERROR: i32 = 2;
pub const D_STRUCT: i32 = 3;
pub const D_UNION: i32 = 4;
pub const D_FN: i32 = 5;
pub const D_CONST: i32 = 6;
pub const D_GLOBAL: i32 = 7;

pub const IDX_ARRAY: i32 = 0;
pub const IDX_SLICE: i32 = 1;
pub const IDX_PTR_ARRAY: i32 = 2;

pub const TY_INVALID: i32 = 0;
pub const TY_VOID: i32 = 1;
pub const TY_BOOL: i32 = 2;
pub const TY_INT: i32 = 3;
pub const TY_FLOAT: i32 = 4;
pub const TY_VEC: i32 = 5;
pub const TY_V128: i32 = 6;
pub const TY_PTR: i32 = 7;
pub const TY_SLICE: i32 = 8;
pub const TY_ARRAY: i32 = 9;
pub const TY_FN: i32 = 10;
pub const TY_STRUCT: i32 = 11;
pub const TY_UNION: i32 = 12;
pub const TY_ERRUNION: i32 = 13;
pub const TY_ERROR: i32 = 14;
pub const TY_PARAM: i32 = 15;
pub const TY_CONSTINT: i32 = 16;
pub const TY_UNTYPED_INT: i32 = 17;
pub const TY_UNTYPED_FLOAT: i32 = 18;
pub const TY_NEVER: i32 = 19;

pub const C_NONE: i32 = 0;
pub const C_EQ: i32 = 1;
pub const C_ORD: i32 = 2;
pub const C_NUM: i32 = 3;
pub const C_INT: i32 = 4;
pub const C_SIGNED: i32 = 5;
pub const C_UNSIGNED: i32 = 6;
pub const C_FLOAT: i32 = 7;

pub const BI_SIZE_OF: i32 = 0;
pub const BI_ALIGN_OF: i32 = 1;
pub const BI_OFFSET_OF: i32 = 2;
pub const BI_MIN_VALUE: i32 = 3;
pub const BI_MAX_VALUE: i32 = 4;
pub const BI_AS: i32 = 5;
pub const BI_INT_CAST: i32 = 6;
pub const BI_TRUNCATE: i32 = 7;
pub const BI_INT_FROM_FLOAT: i32 = 8;
pub const BI_FLOAT_FROM_INT: i32 = 9;
pub const BI_FLOAT_CAST: i32 = 10;
pub const BI_BIT_CAST: i32 = 11;
pub const BI_INT_FROM_PTR: i32 = 12;
pub const BI_PTR_FROM_INT: i32 = 13;
pub const BI_PTR_CAST: i32 = 14;
pub const BI_SLICE_FROM_PTR: i32 = 15;
pub const BI_CAST_SLICE: i32 = 16;
pub const BI_MEM_COPY: i32 = 17;
pub const BI_MEM_SET: i32 = 18;
pub const BI_MEM_EQ: i32 = 19;
pub const BI_MEMORY_SIZE: i32 = 20;
pub const BI_MEMORY_GROW: i32 = 21;
pub const BI_CLZ: i32 = 22;
pub const BI_CTZ: i32 = 23;
pub const BI_POPCOUNT: i32 = 24;
pub const BI_ROTL: i32 = 25;
pub const BI_ROTR: i32 = 26;
pub const BI_ADD_SAT: i32 = 27;
pub const BI_SUB_SAT: i32 = 28;
pub const BI_MUL_SAT: i32 = 29;
pub const BI_ADD_CHECKED: i32 = 30;
pub const BI_SUB_CHECKED: i32 = 31;
pub const BI_MUL_CHECKED: i32 = 32;
pub const BI_MIN: i32 = 33;
pub const BI_MAX: i32 = 34;
pub const BI_ABS: i32 = 35;
pub const BI_SQRT: i32 = 36;
pub const BI_FLOOR: i32 = 37;
pub const BI_CEIL: i32 = 38;
pub const BI_TRUNC: i32 = 39;
pub const BI_NEAREST: i32 = 40;
pub const BI_FMA: i32 = 41;
pub const BI_REM: i32 = 42;
pub const BI_COPYSIGN: i32 = 43;
pub const BI_SPLAT: i32 = 44;
pub const BI_VEC: i32 = 45;
pub const BI_EXTRACT_LANE: i32 = 46;
pub const BI_REPLACE_LANE: i32 = 47;
pub const BI_LOAD: i32 = 48;
pub const BI_STORE: i32 = 49;
pub const BI_SHUFFLE: i32 = 50;
pub const BI_SWIZZLE: i32 = 51;
pub const BI_LANES_EQ: i32 = 52;
pub const BI_LANES_NE: i32 = 53;
pub const BI_LANES_LT: i32 = 54;
pub const BI_LANES_LE: i32 = 55;
pub const BI_LANES_GT: i32 = 56;
pub const BI_LANES_GE: i32 = 57;
pub const BI_SELECT: i32 = 58;
pub const BI_ANY_TRUE: i32 = 59;
pub const BI_ALL_TRUE: i32 = 60;
pub const BI_BITMASK: i32 = 61;
pub const BI_REDUCE_ADD: i32 = 62;
pub const BI_DOT: i32 = 63;
pub const BI_TRAP: i32 = 64;
pub const BI_ATOMIC_LOAD: i32 = 65;
pub const BI_ATOMIC_STORE: i32 = 66;
pub const BI_ATOMIC_ADD: i32 = 67;
pub const BI_ATOMIC_SUB: i32 = 68;
pub const BI_ATOMIC_AND: i32 = 69;
pub const BI_ATOMIC_OR: i32 = 70;
pub const BI_ATOMIC_XOR: i32 = 71;
pub const BI_ATOMIC_XCHG: i32 = 72;
pub const BI_ATOMIC_MIN: i32 = 73;
pub const BI_ATOMIC_MAX: i32 = 74;
pub const BI_ATOMIC_CAS: i32 = 75;
pub const BI_ATOMIC_WAIT: i32 = 76;
pub const BI_ATOMIC_NOTIFY: i32 = 77;
pub const BI_FENCE: i32 = 78;
pub const BI_EXPECT: i32 = 79;
pub const BI_EXPECT_EQ: i32 = 80;
pub const BI_ERROR_NAME: i32 = 81;
pub const BI_MEMORY_DISCARD: i32 = 82;

pub const BI_COUNT: i32 = 83;

pub fn builtin_name(i: i32) []u8 {
    let t = ["size_of", "align_of", "offset_of", "min_value", "max_value", "as", "int_cast", "truncate",
             "int_from_float", "float_from_int", "float_cast", "bit_cast", "int_from_ptr", "ptr_from_int", "ptr_cast",
             "slice_from_ptr", "cast_slice", "mem_copy", "mem_set", "mem_eq", "memory_size", "memory_grow", "clz",
             "ctz", "popcount", "rotl", "rotr", "add_sat", "sub_sat", "mul_sat", "add_checked", "sub_checked",
             "mul_checked", "min", "max", "abs", "sqrt", "floor", "ceil", "trunc", "nearest", "fma", "rem", "copysign",
             "splat", "vec", "extract_lane", "replace_lane", "load", "store", "shuffle", "swizzle", "lanes_eq",
             "lanes_ne", "lanes_lt", "lanes_le", "lanes_gt", "lanes_ge", "select", "any_true", "all_true", "bitmask",
             "reduce_add", "dot", "trap", "atomic_load", "atomic_store", "atomic_add", "atomic_sub", "atomic_and",
             "atomic_or", "atomic_xor", "atomic_xchg", "atomic_min", "atomic_max", "atomic_cas", "atomic_wait",
             "atomic_notify", "fence", "expect", "expect_eq", "error_name", "memory_discard"];
    return t[@int_cast(usize, i)];
}

pub fn builtin_sig(i: i32) []u8 {
    let t = ["t", "t", "te", "t", "t", "te", "te", "te", "te", "te", "te", "te", "e", "te", "te", "ee", "te", "ee",
             "ee", "ee", "", "e", "e", "e", "e", "ee", "ee", "ee", "ee", "ee", "ee", "ee", "ee", "ee", "ee", "e", "e",
             "e", "e", "e", "e", "eee", "ee", "ee", "e", "t*", "ee", "eee", "te", "ee", "eee", "ee", "ee", "ee", "ee",
             "ee", "ee", "ee", "eee", "e", "e", "e", "e", "ee", "", "e", "ee", "ee", "ee", "ee", "ee", "ee", "ee", "ee",
             "ee", "eee", "eee", "ee", "", "e", "ee", "e", "ee"];
    return t[@int_cast(usize, i)];
}

pub fn builtin_lookup(name: []u8) i32 {
    for i in 0..BI_COUNT {
        if u.str_eq(builtin_name(i), name) {
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Type expressions (syntax of types)
// ---------------------------------------------------------------------------
pub struct Field {
    name: []u8,
    te: *mut TypeExpr,
    span: u.Span,
    ty: *mut Type,
}

pub struct TypeExpr {
    kind: i32,
    span: u.Span,
    prim: i32,
    qual: []u8,  // module qualifier (TE_NAMED) or nil
    name: []u8,
    name_span: u.Span,
    args: u.List.[*mut TypeExpr],  // TE_NAMED generic args; TE_FN params
    has_args: bool,
    elem: *mut TypeExpr,  // PTR, SLICE, ARRAY, ERRUNION; TE_FN: return type
    is_mut: bool,
    len: *mut TypeExpr,  // ARRAY: TE_INTLIT or TE_NAMED
    fields: u.List.[Field],
    ival: u64,
}

// ---------------------------------------------------------------------------
// Expressions
// ---------------------------------------------------------------------------
pub struct FieldInit {
    name: []u8,
    span: u.Span,
    value: *mut Expr,
}

pub struct BuiltinArg {
    e: *mut Expr,
    t: *mut TypeExpr,
}

pub struct Expr {
    kind: i32,
    span: u.Span,
    parens: bool,
    // payload
    name: []u8,
    name_span: u.Span,
    ival: u64,
    fval: f64,
    float_text: []u8,
    str: []u8,
    bval: bool,
    op: i32,  // TokKind of operator
    a: *mut Expr,
    b: *mut Expr,
    c: *mut Expr,
    args: u.List.[*mut Expr],
    inits: u.List.[FieldInit],
    bargs: u.List.[BuiltinArg],
    targs: u.List.[*mut TypeExpr],
    // catch
    capture: []u8,
    capture_span: u.Span,
    hkind: i32,
    hexpr: *mut Expr,
    hblock: *mut Stmt,
    capture_local: *mut Local,

    // ---- sema annotations ----
    ty: *mut Type,
    cat: i32,
    local: *mut Local,  // ident -> local
    decl: *mut Decl,    // ident/field -> global/const/fn/type decl; call -> callee fn
    module: *mut Module,
    gargs: []mut *mut Type,  // CAT_FUNC / call: generic args (owner args ++ own args)
    self_mode: i32,
    index: i32,       // field index / variant index / builtin id
    tval: *mut Type,  // CAT_TYPE: the type; E_BUILTIN: type argument
    cval: u.I128,     // folded integer constant (untyped / const refs)
    is_const: bool,   // cval / fval valid
    lit_neg: bool,
}

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------
pub struct MatchArm {
    kind: i32,
    span: u.Span,
    name: []u8,  // variant / error name
    bind: []u8,
    bind_span: u.Span,
    ival: u.I128,
    bval: bool,
    body: *mut Stmt,
    // sema
    bind_local: *mut Local,
    variant: i32,
    error_code: u32,
}

pub struct Stmt {
    kind: i32,
    span: u.Span,
    name: []u8,  // let / for variable
    name_span: u.Span,
    is_mut: bool,
    te: *mut TypeExpr,
    e: *mut Expr,
    e2: *mut Expr,
    e3: *mut Expr,
    op: i32,  // assign op TokKind
    body: *mut Stmt,
    els: *mut Stmt,
    stmts: u.List.[*mut Stmt],
    arms: u.List.[MatchArm],
    // sema
    local: *mut Local,
    diverges: bool,
}

// ---------------------------------------------------------------------------
// Declarations
// ---------------------------------------------------------------------------
pub struct GenericParam {
    name: []u8,
    span: u.Span,
    constraint: []u8,  // nil if unconstrained
    constraint_span: u.Span,
    is_const: bool,
    const_te: *mut TypeExpr,
    ty: *mut Type,  // TY_PARAM
}

pub struct Param {
    name: []u8,
    span: u.Span,
    te: *mut TypeExpr,
    local: *mut Local,
}

pub struct Variant {
    name: []u8,
    span: u.Span,
    te: *mut TypeExpr,  // nil: no payload
    ty: *mut Type,
}

pub struct Local {
    name: []u8,
    span: u.Span,
    ty: *mut Type,
    is_mut: bool,
    is_param: bool,
    addr_taken: bool,
    index: i32,  // within its function
}

pub struct Decl {
    kind: i32,
    span: u.Span,
    name: []u8,
    name_span: u.Span,
    is_pub: bool,
    is_export: bool,
    module: *mut Module,
    gparams: u.List.[GenericParam],
    fields: u.List.[Field],
    variants: u.List.[Variant],
    methods: u.List.[*mut Decl],
    method_map: u.SymMap.[*mut Decl],
    owner: *mut Decl,  // method -> struct/union decl
    params: u.List.[Param],
    ret_te: *mut TypeExpr,
    body: *mut Stmt,
    te: *mut TypeExpr,  // const / global type
    init: *mut Expr,
    is_mut: bool,
    import_path: []u8,
    import_path_span: u.Span,
    imported: *mut Module,
    host_module: []u8,  // D_HOST_FN from `import host "mod"`; nil for extern "C"
    extern_c: bool,     // declared in an extern "C" block
    variadic: bool,     // C varargs: `...` after the fixed parameters
    link_lib: []u8,     // extern "C" "lib": library to link
    abi: []u8,
    is_test: bool,  // `test "name" { }` (a D_FN returning !void)
    test_name: []u8,
    test_trap: i32,  // -1: must not trap; -2: must trap; n >= 0: must trap with Tn
    abi_span: u.Span,
    error_names: u.List.[[]u8],
    error_spans: u.List.[u.Span],

    // ---- sema ----
    state: i32,     // 0 = unresolved, 1 = resolving, 2 = done
    ty: *mut Type,  // fn: TY_FN signature; const/global: value type; struct/union: Self type
    ret_type: *mut Type,
    param_types: []mut *mut Type,
    locals: u.List.[*mut Local],
    all_gparams: []mut *mut Type,  // TY_PARAM types, owner's first
    self_kind: i32,                // for methods: SELF_VALUE (Self), SELF_ADDR (*Self), SELF_PASS (*mut Self); NONE if not a method
    self_mut: bool,
    checked: bool,
    layout_done: bool,
    anon: bool,
    cexpr: *mut Expr,  // const: checked init expression
}

pub struct Module {
    file: *mut u.SourceFile,
    path: []u8,
    decls: u.List.[*mut Decl],
    names: u.SymMap.[*mut Decl],    // top-level names (items, not imports)
    imports: u.SymMap.[*mut Decl],  // alias -> Decl(D_IMPORT)
    pkg: *mut pkg.Package,          // the package this module belongs to, or nil
    checked: bool,
}

// ---------------------------------------------------------------------------
// Semantic types. Types are hash-consed: equal types are the same pointer.
// ---------------------------------------------------------------------------
pub struct Type {
    kind: i32,
    id: i32,
    bits: i32,                // INT / FLOAT; VEC: lane bits
    is_signed: bool,          // INT; VEC with int lanes
    is_size: bool,            // usize / isize
    lanes: i32,               // VEC
    elem: *mut Type,          // PTR, SLICE, ARRAY, ERRUNION, VEC (lane type)
    is_mut: bool,             // PTR, SLICE
    len: i64,                 // ARRAY
    len_param: *mut Type,     // ARRAY whose length is a const generic parameter
    params: []mut *mut Type,  // FN
    ret: *mut Type,
    decl: *mut Decl,  // STRUCT, UNION
    args: []mut *mut Type,
    // PARAM
    owner: *mut Decl,
    pindex: i32,
    constraint: i32,
    is_const: bool,
    const_type: *mut Type,
    name: []u8,
    cval: i64,         // CONSTINT
    has_params: bool,  // mentions a TY_PARAM somewhere
    // layout cache
    size: i64,
    align: i64,
    layout_done: bool,
    backend: *mut void,  // codegen type cache
}

pub struct Target {
    ptr_bits: i32,  // 32 or 64
    is_wasm: bool,
    triple: []u8,
}

// Constructors for zeroed nodes.
pub fn new_te(k: i32, sp: u.Span) *mut TypeExpr {
    let t = u.new.[TypeExpr]();
    t.kind = k;
    t.span = sp;
    return t;
}

pub fn new_expr(k: i32, sp: u.Span) *mut Expr {
    let e = u.new.[Expr]();
    e.kind = k;
    e.span = sp;
    return e;
}

pub fn new_stmt(k: i32, sp: u.Span) *mut Stmt {
    let s = u.new.[Stmt]();
    s.kind = k;
    s.span = sp;
    return s;
}

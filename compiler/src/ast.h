// ast.h - abstract syntax tree. Sema annotates nodes in place.
#ifndef AX_AST_H
#define AX_AST_H

#include "lexer.h"

typedef struct Type Type;
typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Decl Decl;
typedef struct TypeExpr TypeExpr;
typedef struct Module Module;
typedef struct Local Local;

// ---------------------------------------------------------------------------
// Type expressions (syntax of types)
// ---------------------------------------------------------------------------
typedef enum TEKind {
    TE_PRIM,
    TE_NAMED,  // [qual.]name[.[args]]
    TE_PTR,
    TE_SLICE,
    TE_ARRAY,
    TE_FN,
    TE_ANON_STRUCT,
    TE_ERRUNION,
    TE_ERROR,
    TE_SELF,
    TE_INTLIT,  // generic const argument
} TEKind;

typedef struct Field {
    Sym name;
    TypeExpr *te;
    Span span;
    Type *type;
} Field;

typedef Vec(TypeExpr *) TypeExprVec;
typedef Vec(Field) FieldVec;

struct TypeExpr {
    TEKind kind;
    Span span;
    PrimKind prim;
    Sym qual;  // module qualifier (TE_NAMED) or NULL
    Sym name;
    Span name_span;
    TypeExprVec args;  // TE_NAMED generic args; TE_FN params
    bool has_args;
    TypeExpr *elem;  // PTR, SLICE, ARRAY, ERRUNION; TE_FN: return type
    bool mut;
    TypeExpr *len;  // ARRAY: TE_INTLIT or TE_NAMED
    FieldVec fields;
    uint64_t ival;
};

// ---------------------------------------------------------------------------
// Expressions
// ---------------------------------------------------------------------------
typedef enum ExprKind {
    E_IDENT,
    E_SELF,
    E_INT,
    E_FLOAT,
    E_STRING,
    E_BOOL,
    E_STRUCT_LIT,   // a.{...} (a may be NULL for anonymous .{...})
    E_ARRAY_LIT,    // [a, b, c]
    E_ARRAY_REPEAT, // [a; b]
    E_ERROR_VAL,    // error.Name
    E_BUILTIN,
    E_UNARY,
    E_BINARY,
    E_CATCH,
    E_FIELD,  // a.name
    E_DEREF,  // a.*
    E_CALL,
    E_INDEX,
    E_SLICE,
    E_TRY,    // a!
    E_APPLY,  // a.[targs]
    E_CONV,   // implicit conversion inserted by sema (op = ConvKind)
} ExprKind;

typedef enum ConvKind { CONV_NOOP, CONV_WRAP_OK, CONV_WRAP_ERR } ConvKind;

// E_CALL kinds (stored in Expr.op after sema)
typedef enum CallKind { CALLK_DIRECT = 0, CALLK_INDIRECT = 1, CALLK_VARIANT = 2 } CallKind;

// E_FIELD variants (stored in Expr.op after sema)
typedef enum FieldKind {
    FK_STRUCT,       // struct field (index)
    FK_SLICE_LEN,
    FK_SLICE_PTR,
    FK_ARRAY_LEN,
    FK_VARIANT_UNIT, // U.Variant without payload (value)
    FK_CONST,        // module.CONST or similar folded value
    FK_DECL,         // module.name referring to a global or function
} FieldKind;

// Value category of an expression after sema.
typedef enum ExprCat {
    CAT_VALUE,
    CAT_TYPE,       // expression names a type (for T.init, T.{...})
    CAT_MODULE,     // imported module alias
    CAT_FUNC,       // reference to a (possibly generic) function
    CAT_METHOD,     // receiver.method (only valid as callee)
    CAT_VARIANT,    // U.Variant with payload (only valid as callee)
    CAT_GENERIC,    // uninstantiated generic type or function
} ExprCat;

typedef enum HandlerKind { H_VALUE, H_RETURN, H_BREAK, H_CONTINUE, H_BLOCK } HandlerKind;

typedef enum SelfMode { SELF_NONE, SELF_VALUE, SELF_ADDR, SELF_PASS, SELF_DEREF } SelfMode;

typedef struct FieldInit {
    Sym name;
    Span span;
    Expr *value;
} FieldInit;

typedef struct BuiltinArg {
    Expr *e;
    TypeExpr *t;
} BuiltinArg;

typedef Vec(Expr *) ExprVec;

struct Expr {
    ExprKind kind;
    Span span;
    bool parens;
    // payload
    Sym name;
    Span name_span;
    uint64_t ival;
    double fval;
    const char *float_text;
    const char *str;
    uint32_t slen;
    bool bval;
    int op;  // TokKind of operator
    Expr *a, *b, *c;
    ExprVec args;
    Vec(FieldInit) inits;
    Vec(BuiltinArg) bargs;
    TypeExprVec targs;
    // catch
    Sym capture;
    Span capture_span;
    HandlerKind hkind;
    Expr *hexpr;
    Stmt *hblock;
    Local *capture_local;

    // ---- sema annotations ----
    Type *type;
    ExprCat cat;
    Local *local;  // ident -> local
    Decl *decl;    // ident/field -> global/const/fn/type decl; call -> callee fn
    Module *module;
    Type **gargs;  // CAT_FUNC / call: generic args (owner args ++ own args)
    int ngargs;
    SelfMode self_mode;
    int index;          // field index / variant index / builtin id
    Type *tval;         // CAT_TYPE: the type; E_BUILTIN: type argument
    i128 cval;          // folded integer constant (untyped / const refs)
    bool is_const;      // cval / fval valid
    bool lit_neg;       // literal arithmetic produced a negative value
};

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------
typedef enum StmtKind {
    S_LET,
    S_EXPR,
    S_ASSIGN,
    S_DISCARD,
    S_IF,
    S_WHILE,
    S_LOOP,
    S_FOR,
    S_MATCH,
    S_RETURN,
    S_BREAK,
    S_CONTINUE,
    S_DEFER,
    S_BLOCK,
} StmtKind;

typedef enum PatKind { P_VARIANT, P_ERROR, P_INT, P_BOOL, P_ELSE } PatKind;

typedef struct MatchArm {
    PatKind kind;
    Span span;
    Sym name;  // variant / error name
    Sym bind;
    Span bind_span;
    i128 ival;
    bool bval;
    Stmt *body;
    // sema
    Local *bind_local;
    int variant;
    uint32_t error_code;
} MatchArm;

typedef Vec(Stmt *) StmtVec;

struct Stmt {
    StmtKind kind;
    Span span;
    Sym name;  // let / for variable
    Span name_span;
    bool mut;
    TypeExpr *te;
    Expr *e, *e2, *e3;
    int op;  // assign op TokKind
    Stmt *body, *els;
    StmtVec stmts;
    Vec(MatchArm) arms;
    // sema
    Local *local;
    bool diverges;  // block never completes normally
    // codegen
    bool no_overflow;  // `x += e` that cannot overflow (codegen.c, mark_bounded_sums)
};

// ---------------------------------------------------------------------------
// Declarations
// ---------------------------------------------------------------------------
typedef enum DeclKind { D_IMPORT, D_HOST_FN, D_ERROR, D_STRUCT, D_UNION, D_ENUM, D_FN, D_CONST, D_GLOBAL } DeclKind;

typedef struct GenericParam {
    Sym name;
    Span span;
    Sym constraint;  // NULL if unconstrained
    Span constraint_span;
    bool is_const;
    TypeExpr *const_te;
    Type *type;  // TY_PARAM
} GenericParam;

typedef struct Param {
    Sym name;
    Span span;
    TypeExpr *te;
    Local *local;
} Param;

typedef struct Variant {
    Sym name;
    Span span;
    TypeExpr *te;  // NULL: no payload
    Type *type;
    // enum members
    bool has_value;  // `= value` given
    i128 value;      // after sema: the member's value
    Span value_span;
} Variant;

struct Local {
    Sym name;
    Span span;
    Type *type;
    bool mut;
    bool is_param;
    bool addr_taken;
    int index;  // within its function
};

struct Decl {
    DeclKind kind;
    Span span;
    Sym name;
    Span name_span;
    bool is_pub, is_export;
    Module *module;
    Vec(GenericParam) gparams;
    FieldVec fields;
    Vec(Variant) variants;
    Vec(Decl *) methods;
    SymMap method_map;
    Decl *owner;  // method -> struct/union/enum decl
    Vec(Param) params;
    TypeExpr *ret_te;
    Stmt *body;
    TypeExpr *te;  // const / global type; enum representation type (NULL: default)
    Expr *init;
    bool mut;
    bool tls;  // D_GLOBAL: threadlocal (one per thread)
    Sym import_path;
    Span import_path_span;
    Module *imported;
    Sym host_module;   // D_HOST_FN from `import host "mod"`; NULL for extern "C"
    bool extern_c;     // declared in an extern "C" block
    bool variadic;     // C varargs: `...` after the fixed parameters
    Sym link_lib;      // extern "C" "lib": library to link
    Sym abi;
    bool is_inline;    // `inline fn`: always inlined (spec §9.5)
    Span inline_span;
    Vec(Decl *) inline_callees;  // inline functions this one calls directly (sema)
    Vec(Span) inline_call_spans;
    int inline_mark;  // sema: visited by the cycle check
    bool is_test;      // `test "name" { }` (a D_FN returning !void)
    Sym test_name;
    int test_trap;     // -1: must not trap; -2: must trap; n >= 0: must trap with Tn
    Span abi_span;
    Vec(Sym) error_names;
    Vec(Span) error_spans;
    // fn attribute: @features("a,b") or @multiversion("a,b", "c") after the return type
    int attr;  // ATTR_*
    Span attr_span;
    Vec(Sym) attr_args;
    Vec(Span) attr_arg_spans;

    // ---- sema ----
    int state;          // 0 = unresolved, 1 = resolving, 2 = done
    Type *type;         // fn: TY_FN signature; const/global: value type; struct/union: Self type
    Type *ret_type;
    Type **param_types;
    Vec(Local *) locals;
    int nall_gparams;   // owner's + own generic params
    Type **all_gparams; // TY_PARAM types, owner's first
    SelfMode self_kind;  // for methods: SELF_VALUE (Self), SELF_ADDR (*Self), SELF_PASS (*mut Self); NONE if not a method
    bool self_mut;
    bool checked;
    bool layout_done;
    bool anon;
    Expr *cexpr;  // const: checked init expression
    uint64_t *feat_sets;  // fn with an attribute: the extensions each version adds (closed under implication)
};

enum {
    ATTR_NONE,
    ATTR_FEATURES,      // @features: compiled with more extensions; traps (T0013) without them
    ATTR_MULTIVERSION,  // @multiversion: one version per set, chosen at run time
};

struct Module {
    SourceFile *file;
    Sym path;
    Vec(Decl *) decls;
    SymMap names;    // top-level names (items, not imports)
    SymMap imports;  // alias -> Decl(D_IMPORT)
    struct Package *pkg;  // the package this module belongs to, or NULL
    bool checked;
};

// Parser entry point. Returns a module with decls; errors via diag.
Module *parse_module(SourceFile *f);

#endif

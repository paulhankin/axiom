// sema_int.h - internals shared by the sema_*.c files.
#ifndef AX_SEMA_INT_H
#define AX_SEMA_INT_H

#include "builtins.h"
#include "diag.h"
#include "sema.h"

// Context for resolving type expressions: which generic parameters and
// which `Self` are in scope.
typedef struct TypeCtx {
    Module *mod;
    Decl *gowner;  // struct/union whose generic params are in scope
    Decl *gfn;     // function whose generic params are in scope
    Type *self;    // `Self`, or NULL
} TypeCtx;

typedef struct ScopeEntry {
    Sym name;
    Local *local;
} ScopeEntry;

typedef struct FnCtx {
    TypeCtx tc;
    Decl *fn;   // NULL when checking a const/global initializer
    Type *ret;  // declared return type
    Vec(ScopeEntry) scope;
    Vec(int) marks;
    int loop_depth;
    int defer_depth;
    int defer_loop_base;  // loop_depth when the innermost defer began
    Vec(bool) loop_breaks;  // per enclosing loop: has a break targeting it
    bool const_ctx;
} FnCtx;

extern FnCtx *C;

// sema_decl.c
Type *resolve_type(TypeExpr *te, TypeCtx *tc);
void resolve_decl(Decl *d);
Decl *module_lookup(Module *m, Sym name);
Type *lookup_gparam(TypeCtx *tc, Sym name);
Decl *find_method(Decl *owner, Sym name);
int find_variant(Decl *u, Sym name);
int find_field(Decl *s, Sym name);
bool check_generic_args(Decl *d, Type **args, int n, Span sp, TypeExprVec *tes);
Type *make_self_type(Decl *d);
Type **decl_gparam_types(Decl *d);  // own params of a struct/union/fn

// sema_expr.c
Type *check_expr(Expr *e, Type *expected);
void coerce(Expr **pe, Type *target);
Type *check_expr_to(Expr **pe, Type *target);
bool is_place(Expr *e);
bool is_mut_place(Expr *e);
void mark_addr_taken(Expr *e);
void check_value(Expr *e);  // reports if e is not a value (type/module/...)
Type *finalize_untyped(Expr **pe);  // untyped literal without context -> E0502
Type *builtin_check(Expr *e, Type *expected);
bool fits_type(i128 v, Type *t);

// sema_stmt.c
bool check_block(Stmt *b);
bool check_stmt(Stmt *s);
void check_fn_body(Decl *fn);
Local *declare_local(Sym name, Span sp, Type *t, bool mut, bool is_param);
void scope_push(void);
void scope_pop(void);
Local *scope_lookup(Sym name);

// sema_target.c
int target_name_value(int id, const char *name);
void check_fn_attr(Decl *f);
void gate_fn(Decl *f);
int fn_versions(Decl *f, uint64_t *out);

// helpers
static inline Span expr_span(Expr *e) { return e->span; }

#endif

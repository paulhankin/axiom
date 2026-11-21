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

typedef struct LoopInfo {
    Sym label;
    bool has_break;
    int local_base;         // C->locals.len at loop entry
    bool *entry_moved;      // length local_base
    Span *entry_spans;      // length local_base
    bool *break_moved;      // length local_base
    Span *break_spans;      // length local_base
    Local **break_origins;        // length local_base
    uint64_t *break_origin_fields;  // length local_base
    bool break_snap_init;
} LoopInfo;

typedef struct MoveSnap {
    int len;
    bool *moved;
    Span *spans;
    Local **origins;
    uint64_t *origin_fields;
} MoveSnap;

typedef struct FnCtx {
    TypeCtx tc;
    Decl *fn;   // NULL when checking a const/global initializer
    Type *ret;  // declared return type
    Vec(ScopeEntry) scope;
    Vec(int) marks;
    int loop_depth;
    int defer_depth;
    int defer_loop_base;  // loop_depth when the innermost defer began
    Vec(LoopInfo) loops;  // enclosing loops
    Vec(Local *) locals;  // all locals created in this context
    bool flow_dead;
    bool suppress_moved_ident;
    bool const_ctx;
    bool in_return;
} FnCtx;

extern FnCtx *C;

// sema_decl.c
Type *resolve_type(TypeExpr *te, TypeCtx *tc);
Type *resolve_generic_arg(TypeExpr *te, TypeCtx *tc);
void resolve_decl(Decl *d);
Decl *resolve_set_te(TypeExpr *te, TypeCtx *tc);
bool set_has(Decl *set, Sym name);
bool error_fits(Span sp, Decl *a, Decl *b);
void try_fits(Span sp, Decl *a, Decl *b);
Decl *expected_set(Type *t);
bool union_fits(Span sp, Type *s, Type *t);
bool fn_widens(Type *s, Type *t);
bool type_is_nocopy(Type *t);

// sema_stmt.c: a break or continue at `sp` naming the loop `label` (or
// NULL): the loops it leaves beyond the innermost, or -1 after an error.
int check_jump_target(Sym label, Span sp, bool is_break);
Decl *module_lookup(Module *m, Sym name);
Type *lookup_gparam(TypeCtx *tc, Sym name);
Decl *find_method(Decl *owner, Sym name);
int find_variant(Decl *u, Sym name);
int find_field(Decl *s, Sym name);
bool check_generic_args(Decl *d, Type **args, int n, Span sp, TypeExprVec *tes);
bool check_generic_args_ctx(Decl *d, Type **owner_args, int nowner, Type **args, int n, Span sp, TypeExprVec *tes);
Type *make_self_type(Decl *d);
Type **decl_gparam_types(Decl *d);  // own params of a struct/union/fn
const char *param_constraint_str(const Type *t);

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
void check_consume_nocopy(Expr *e, bool allow_local_move);
Local *place_local(Expr *e);
Local *place_origin(Expr *e);
Local *expr_local_origin(Expr *e);
Local *expr_local_origin_mask(Expr *e, uint64_t *mask_out);

// sema_stmt.c
bool check_block(Stmt *b);
bool check_stmt(Stmt *s);
void check_fn_body(Decl *fn);
Local *declare_local(Sym name, Span sp, Type *t, bool mut, bool is_param);
void scope_push(void);
void scope_pop(void);
Local *scope_lookup(Sym name);
MoveSnap move_snap_take(void);
void move_snap_restore(const MoveSnap *s);
void move_snap_join(const MoveSnap *s);
void move_snap_free(MoveSnap *s);

// sema_target.c
int target_name_value(int id, const char *name);
void check_fn_attr(Decl *f);
void gate_fn(Decl *f);
int fn_versions(Decl *f, uint64_t *out);

// helpers
static inline Span expr_span(Expr *e) { return e->span; }

#endif

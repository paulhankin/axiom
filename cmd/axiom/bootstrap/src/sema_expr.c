// sema_expr.c - expression type checking.
#include <math.h>

#include "sema_int.h"

enum { IDX_ARRAY, IDX_SLICE, IDX_PTR_ARRAY };

static Type *check_operand(Expr *e, Type *expected);

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static Expr *mkconv(Expr *e, ConvKind k, Type *t) {
    Expr *c = NEW(Expr);
    c->kind = E_CONV;
    c->span = e->span;
    c->a = e;
    c->op = k;
    c->type = t;
    c->cat = CAT_VALUE;
    return c;
}

static void type_mismatch(Expr *e, Type *expected, Type *found) {
    if (expected->kind == TY_INVALID || found->kind == TY_INVALID) return;
    Diag *d = diag_error(e->span, "E0412", "type mismatch: expected %s, found %s", type_str(expected), type_str(found));
    diag_types(d, type_str(expected), type_str(found));
    if (expected->kind == TY_VEC && found == expected->elem) {
        SourceFile *f = g_files.data[e->span.file];
        const char *txt = arena_strndup(f->text + e->span.start, e->span.end - e->span.start);
        DiagFix *fx = diag_fix(d, "broadcast the scalar to all lanes", "machine");
        diag_fix_edit(fx, e->span, arena_printf("@splat(%s)", txt));
    } else if (expected->kind == TY_INT && found->kind == TY_INT) {
        SourceFile *f = g_files.data[e->span.file];
        const char *txt = arena_strndup(f->text + e->span.start, e->span.end - e->span.start);
        bool lossless = (found->is_signed == expected->is_signed && found->bits < expected->bits) ||
                        (!found->is_signed && expected->is_signed && found->bits < expected->bits);
        DiagFix *fx = diag_fix(d, lossless ? "convert explicitly (lossless)" : "convert explicitly (checked at run time)",
                               "suggestion");
        diag_fix_edit(fx, e->span, arena_printf(lossless ? "@as(%s, %s)" : "@int_cast(%s, %s)", type_str(expected), txt));
    }
}

bool fits_type(i128 v, Type *t) {
    if (t->kind != TY_INT) return true;
    i128 lo, hi;
    int_range(t, &lo, &hi);
    return v >= lo && v <= hi;
}

static const char *i128_str(i128 v) {
    char buf[64];
    int n = 0;
    bool neg = v < 0;
    unsigned __int128 u = neg ? (unsigned __int128)(-(v + 1)) + 1 : (unsigned __int128)v;
    do {
        buf[n++] = (char)('0' + (int)(u % 10));
        u /= 10;
    } while (u);
    if (neg) buf[n++] = '-';
    char *s = arena_alloc((size_t)n + 1);
    for (int i = 0; i < n; i++) s[i] = buf[n - 1 - i];
    return s;
}

// Literal ranges allowed for a generic parameter type (spec §10.5).
static bool lit_ok_for_param(Expr *e, Type *p) {
    if (e->type->kind == TY_UNTYPED_FLOAT) return p->constraint == C_FLOAT;
    i128 v = e->cval;
    switch (p->constraint) {
    case C_NUM:
    case C_INT: return v >= 0 && v <= 127;
    case C_SIGNED: return v >= -128 && v <= 127;
    case C_UNSIGNED: return v >= 0 && v <= 255;
    case C_FLOAT: return v >= -(1 << 24) && v <= (1 << 24);
    default: return false;
    }
}

// Whether constant e has an integer value: an untyped integer, or one
// coerced to a type parameter.
static bool int_valued(Expr *e) {
    if (e->type && e->type->kind == TY_UNTYPED_INT) return true;
    if (e->kind == E_INT) return true;
    if (e->kind == E_UNARY) return int_valued(e->a);
    if (e->kind == E_BINARY) return int_valued(e->a) && int_valued(e->b);
    return false;
}

// The value of a constant float expression of type f32 (or untyped,
// becoming f32): its literals are rounded to f32, a decimal one directly
// rather than through f64, and its operations are f32 operations (§6.1).
float const_f32(Expr *e) {
    if (int_valued(e)) return (float)e->cval;
    if (e->kind == E_FLOAT) return strtof(e->float_text, NULL);
    if (e->kind == E_UNARY && e->op == TK_MINUS && e->a->is_const) return -const_f32(e->a);
    if (e->kind == E_BINARY && e->a->is_const && e->b->is_const) {
        float a = const_f32(e->a), b = const_f32(e->b);
        switch (e->op) {
        case TK_PLUS: return a + b;
        case TK_MINUS: return a - b;
        case TK_STAR: return a * b;
        case TK_SLASH: return a / b;
        default: break;
        }
    }
    return (float)e->fval;
}

void coerce(Expr **pe, Type *target) {
    Expr *e = *pe;
    Type *s = e->type;
    if (!s || !target) return;
    if (s->kind == TY_NEVER) {
        e->tval = target;
        return;
    }
    if (s == target || s->kind == TY_INVALID || target->kind == TY_INVALID) return;
    if (s->kind == TY_UNTYPED_INT) {
        if (target->kind == TY_INT) {
            if (!fits_type(e->cval, target)) {
                i128 lo, hi;
                int_range(target, &lo, &hi);
                diag_error(e->span, "E0503", "literal %s does not fit in %s (range %s..%s)", i128_str(e->cval),
                           type_str(target), i128_str(lo), i128_str(hi));
            }
            e->type = target;
            return;
        }
        if (target->kind == TY_FLOAT) {
            e->fval = (double)e->cval;
            if (target->bits == 32) e->fval = (double)(float)e->cval;
            e->type = target;
            return;
        }
        if (target->kind == TY_PARAM && !target->is_const) {
            if (!lit_ok_for_param(e, target))
                diag_error(e->span, "E0461",
                           "literal %s is not representable in every type satisfying %s; convert explicitly",
                           i128_str(e->cval), constraint_name(target->constraint));
            e->fval = (double)e->cval;
            e->type = target;
            return;
        }
        if (target->kind == TY_ERRUNION && target->elem->kind != TY_VOID) {
            coerce(pe, target->elem);
            *pe = mkconv(*pe, CONV_WRAP_OK, target);
            return;
        }
        type_mismatch(e, target, s);
        e->type = t_invalid;
        return;
    }
    if (s->kind == TY_UNTYPED_FLOAT) {
        if (target->kind == TY_FLOAT) {
            if (target->bits == 32 && e->is_const) e->fval = (double)const_f32(e);
            e->type = target;
            return;
        }
        if (target->kind == TY_PARAM && !target->is_const) {
            if (!lit_ok_for_param(e, target))
                diag_error(e->span, "E0461", "float literal is not allowed for a parameter constrained by %s",
                           constraint_name(target->constraint));
            e->type = target;
            return;
        }
        if (target->kind == TY_ERRUNION && target->elem->kind == TY_FLOAT) {
            coerce(pe, target->elem);
            *pe = mkconv(*pe, CONV_WRAP_OK, target);
            return;
        }
        type_mismatch(e, target, s);
        e->type = t_invalid;
        return;
    }
    // mutability weakening
    if ((s->kind == TY_SLICE || s->kind == TY_PTR) && target->kind == s->kind && s->mut && !target->mut &&
        s->elem == target->elem) {
        *pe = mkconv(e, CONV_NOOP, target);
        return;
    }
    // an error of one set where one of another (or any error) is needed
    if (s->kind == TY_ERROR && target->kind == TY_ERROR) {
        if (error_fits(e->span, s->decl, target->decl)) *pe = mkconv(e, CONV_NOOP, target);
        return;
    }
    // a narrower error union where a wider one is expected
    if (s->kind == TY_ERRUNION && target->kind == TY_ERRUNION && s->elem == target->elem) {
        if (union_fits(e->span, s, target)) *pe = mkconv(e, CONV_NOOP, target);
        return;
    }
    // a function whose results widen so
    if (s->kind == TY_FN && target->kind == TY_FN && fn_widens(s, target)) {
        *pe = mkconv(e, CONV_NOOP, target);
        return;
    }
    if (target->kind == TY_ERRUNION) {
        if (s->kind == TY_ERROR) {
            if (error_fits(e->span, s->decl, target->decl)) *pe = mkconv(e, CONV_WRAP_ERR, target);
            return;
        }
        if (s->kind != TY_ERRUNION && target->elem->kind != TY_VOID) {
            coerce(pe, target->elem);
            if ((*pe)->type == target->elem || (*pe)->type->kind == TY_INVALID) {
                *pe = mkconv(*pe, CONV_WRAP_OK, target);
                (*pe)->type = target;
            }
            return;
        }
    }
    type_mismatch(e, target, s);
}

Type *check_expr_to(Expr **pe, Type *target) {
    check_expr(*pe, target);
    coerce(pe, target);
    return (*pe)->type;
}

Type *finalize_untyped(Expr **pe) {
    Expr *e = *pe;
    if (e->type && ty_is_untyped(e->type)) {
        Diag *d = diag_error(e->span, "E0502", "cannot infer the type of this literal; add a type annotation");
        (void)d;
        e->type = t_invalid;
    }
    return e->type;
}

static const char *cat_name(ExprCat c) {
    switch (c) {
    case CAT_TYPE: return "a type";
    case CAT_MODULE: return "a module";
    case CAT_FUNC: return "a function";
    case CAT_METHOD: return "a method";
    case CAT_VARIANT: return "a union variant constructor";
    case CAT_GENERIC: return "a generic declaration";
    default: return "a value";
    }
}

static void func_to_value(Expr *e) {
    Decl *fn = e->decl;
    if (fn->kind == D_HOST_FN) {
        e->cat = CAT_VALUE;
        e->type = fn->type;
        return;
    }
    if (e->ngargs < fn->nall_gparams) {
        Diag *d = diag_error(e->span, "E0466", "generic function '%s' must be instantiated before use as a value: %s.[...]",
                             fn->name, fn->name);
        diag_note(d, fn->name_span, "'%s' declared here", fn->name);
        e->cat = CAT_VALUE;
        e->type = t_invalid;
        return;
    }
    e->cat = CAT_VALUE;
    e->type = type_subst(fn->type, fn->all_gparams, e->gargs, fn->nall_gparams);
}

void check_value(Expr *e) {
    if (e->cat == CAT_VALUE) return;
    if (e->cat == CAT_FUNC) {
        func_to_value(e);
        return;
    }
    diag_error(e->span, "E0405", "expected a value, found %s", cat_name(e->cat));
    e->cat = CAT_VALUE;
    e->type = t_invalid;
}

Type *check_expr(Expr *e, Type *expected) {
    check_operand(e, expected);
    check_value(e);
    if (!e->type) e->type = t_invalid;
    return e->type;
}

// ---------------------------------------------------------------------------
// Places
// ---------------------------------------------------------------------------
bool is_place(Expr *e) {
    switch (e->kind) {
    case E_IDENT: return e->local != NULL || (e->decl && e->decl->kind == D_GLOBAL);
    case E_DEREF: return true;
    case E_FIELD:
        if (e->op == FK_DECL) return e->decl && e->decl->kind == D_GLOBAL;
        if (e->op != FK_STRUCT) return false;
        if (e->self_mode == SELF_DEREF) return true;
        return is_place(e->a);
    case E_INDEX: return e->op != IDX_ARRAY || is_place(e->a);
    default: return false;
    }
}

bool is_mut_place(Expr *e) {
    switch (e->kind) {
    case E_IDENT: return e->local ? e->local->mut : (e->decl && e->decl->kind == D_GLOBAL && e->decl->mut);
    case E_DEREF: return e->a->type->kind == TY_PTR && e->a->type->mut;
    case E_FIELD:
        if (e->op == FK_DECL) return e->decl && e->decl->kind == D_GLOBAL && e->decl->mut;
        if (e->op != FK_STRUCT) return false;
        if (e->self_mode == SELF_DEREF) return e->a->type->mut;
        return is_mut_place(e->a);
    case E_INDEX:
        if (e->op == IDX_ARRAY) return is_mut_place(e->a);
        return e->a->type->mut;
    default: return false;
    }
}

void mark_addr_taken(Expr *e) {
    switch (e->kind) {
    case E_IDENT:
        if (e->local) e->local->addr_taken = true;
        return;
    case E_FIELD:
        if (e->self_mode != SELF_DEREF) mark_addr_taken(e->a);
        return;
    case E_INDEX:
        if (e->op == IDX_ARRAY) mark_addr_taken(e->a);
        return;
    default: return;
    }
}

static Local *root_local(Expr *e) {
    for (;;) {
        if (e->kind == E_IDENT) return e->local;
        if ((e->kind == E_FIELD && e->self_mode != SELF_DEREF) || (e->kind == E_INDEX && e->op == IDX_ARRAY)) e = e->a;
        else return NULL;
    }
}

static void not_mutable(Expr *e, const char *what) {
    Diag *d = diag_error(e->span, "E0701", "cannot %s: this place is not mutable", what);
    Local *l = root_local(e);
    if (l && !l->is_param) {
        diag_note(d, l->span, "'%s' is declared without 'mut'", l->name);
        DiagFix *fx = diag_fix(d, "declare the variable as mutable", "suggestion");
        diag_fix_edit(fx, (Span){l->span.file, l->span.start, l->span.start}, "mut ");
    } else if (l && l->is_param) {
        diag_note(d, l->span, "parameters are immutable; copy '%s' into a 'let mut' first", l->name);
    }
}

// ---------------------------------------------------------------------------
// Constant folding
// ---------------------------------------------------------------------------
static i128 wrap_to(i128 v, Type *t) {
    int b = t->bits;
    unsigned __int128 m = ((unsigned __int128)1 << b) - 1;
    unsigned __int128 u = (unsigned __int128)v & m;
    if (t->is_signed && (u >> (b - 1)) & 1) return (i128)u - ((i128)1 << b);
    return (i128)u;
}

static const i128 UNTYPED_LIMIT = (i128)1 << 100;

// Folds integer op. t == NULL for untyped. Returns false on error (reported).
static bool fold_int(int op, i128 a, i128 b, Type *t, i128 *out, Span sp) {
    i128 r = 0;
    switch (op) {
    case TK_PLUS: r = a + b; break;
    case TK_MINUS: r = a - b; break;
    case TK_STAR:
        if (a != 0 && (b > UNTYPED_LIMIT || b < -UNTYPED_LIMIT || a > UNTYPED_LIMIT || a < -UNTYPED_LIMIT)) {
            diag_error(sp, "E0530", "overflow in constant expression");
            return false;
        }
        r = a * b;
        break;
    case TK_SLASH:
    case TK_PERCENT:
        if (b == 0) {
            diag_error(sp, "E0531", "division by zero in constant expression");
            return false;
        }
        r = op == TK_SLASH ? a / b : a % b;
        break;
    case TK_PLUS_PCT: r = a + b; if (t) r = wrap_to(r, t); break;
    case TK_MINUS_PCT: r = a - b; if (t) r = wrap_to(r, t); break;
    case TK_STAR_PCT: r = t ? wrap_to(wrap_to(a, t) * wrap_to(b, t), t) : a * b; break;
    case TK_AMP: r = a & b; if (t) r = wrap_to(r, t); break;
    case TK_PIPE: r = a | b; if (t) r = wrap_to(r, t); break;
    case TK_CARET: r = a ^ b; if (t) r = wrap_to(r, t); break;
    case TK_SHL:
        if (t) {
            r = b >= t->bits ? 0 : wrap_to(a << b, t);
        } else {
            if (b > 100) {
                diag_error(sp, "E0530", "overflow in constant expression");
                return false;
            }
            r = a << b;
        }
        break;
    case TK_SHR:
        if (b >= 120) r = a < 0 ? -1 : 0;
        else r = a >> b;  // arithmetic: floor
        break;
    default: return false;
    }
    if (t && !fits_type(r, t)) {
        diag_error(sp, "E0530", "overflow in constant expression: %s does not fit in %s", i128_str(r), type_str(t));
        return false;
    }
    if (!t && (r > UNTYPED_LIMIT || r < -UNTYPED_LIMIT)) {
        diag_error(sp, "E0530", "constant expression is too large");
        return false;
    }
    *out = r;
    return true;
}

// ---------------------------------------------------------------------------
// Operators
// ---------------------------------------------------------------------------
static bool is_cmp_op(int op) {
    return op == TK_EQEQ || op == TK_NE || op == TK_LT || op == TK_LE || op == TK_GT || op == TK_GE;
}
static bool is_shift_op(int op) { return op == TK_SHL || op == TK_SHR; }

static bool param_has(Type *t, Constraint c) { return t->kind == TY_PARAM && type_satisfies(t, c); }

// Validate that arithmetic/bitwise operator `op` applies to operand type t.
static bool check_arith_op(int op, Type *t, Span sp) {
    if (t->kind == TY_INVALID || t->kind == TY_NEVER) return true;
    const char *opn = tok_kind_name((TokKind)op);
    switch (op) {
    case TK_PLUS:
    case TK_MINUS:
    case TK_STAR:
    case TK_SLASH:
        if (ty_is_num(t) || ty_is_untyped(t) || ty_is_vec_float(t) || param_has(t, C_NUM) || param_has(t, C_FLOAT_VEC))
            return true;
        if (ty_is_vec_int(t)) {
            // lane-wise and wrapping, in every overflow mode (§13.1)
            if (op == TK_SLASH) diag_error(sp, "E0811", "integer vectors do not support '/'");
            else if (op == TK_STAR && t->elem->bits == 8) diag_error(sp, "E0811", "8-bit lane vectors have no multiplication");
            else return true;
            return false;
        }
        break;
    case TK_PERCENT:
        if (t->kind == TY_INT || t->kind == TY_UNTYPED_INT || param_has(t, C_INT)) return true;
        if (t->kind == TY_FLOAT || t->kind == TY_UNTYPED_FLOAT) {
            diag_error(sp, "E0521", "'%%' is not defined for floats; use @rem");
            return false;
        }
        break;
    case TK_PLUS_PCT:
    case TK_MINUS_PCT:
    case TK_STAR_PCT:
        if (t->kind == TY_INT || t->kind == TY_UNTYPED_INT || param_has(t, C_INT)) return true;
        if (ty_is_vec_int(t)) {
            if (op == TK_STAR_PCT && t->elem->bits == 8) {
                diag_error(sp, "E0811", "8-bit lane vectors have no multiplication");
                return false;
            }
            return true;
        }
        break;
    case TK_AMP:
    case TK_PIPE:
    case TK_CARET:
        if (t->kind == TY_INT || t->kind == TY_UNTYPED_INT || ty_is_vec_int(t) || t->kind == TY_V128 ||
            param_has(t, C_INT))
            return true;
        break;
    }
    if (t->kind == TY_PARAM)
        diag_error(sp, "E0461", "operator '%s' is not permitted on '%s' (constraint %s)", opn, t->name,
                   constraint_name(t->constraint));
    else
        diag_error(sp, "E0513", "operator '%s' cannot be applied to %s", opn, type_str(t));
    return false;
}

static bool check_cmp_op(int op, Type *t, Span sp) {
    if (t->kind == TY_INVALID || t->kind == TY_NEVER) return true;
    bool eq = op == TK_EQEQ || op == TK_NE;
    if (ty_is_num(t) || ty_is_untyped(t)) return true;
    if (t->kind == TY_PARAM) {
        if (type_satisfies(t, eq ? C_EQ : C_ORD)) return true;
        diag_error(sp, "E0461", "operator '%s' is not permitted on '%s' (constraint %s)", tok_kind_name((TokKind)op),
                   t->name, constraint_name(t->constraint));
        return false;
    }
    if (eq && (t->kind == TY_BOOL || t->kind == TY_PTR || t->kind == TY_ERROR || t->kind == TY_FN || t->kind == TY_ENUM))
        return true;
    if (t->kind == TY_VEC || t->kind == TY_V128)
        diag_error(sp, "E0810", "vectors cannot be compared with '%s'; use @lanes_eq, @lanes_lt, ...",
                   tok_kind_name((TokKind)op));
    else
        diag_error(sp, "E0513", "operator '%s' cannot be applied to %s", tok_kind_name((TokKind)op), type_str(t));
    return false;
}

// Checks the right operand of a shift; returns false on error.
static bool check_shift_count(Expr **pb) {
    Type *r = check_expr(*pb, NULL);
    if (r->kind == TY_UNTYPED_INT) {
        if ((*pb)->cval < 0) {
            diag_error((*pb)->span, "E0525", "shift count must not be negative");
            return false;
        }
        coerce(pb, t_u32);
        return true;
    }
    if (r->kind == TY_INVALID) return true;
    if ((r->kind == TY_INT && !r->is_signed) || param_has(r, C_UNSIGNED)) return true;
    if (ty_is_vec_int(r) && !r->elem->is_signed) return true;  // per lane (spec §13.1)
    if (r->kind == TY_INT || param_has(r, C_SIGNED) || ty_is_vec_int(r)) {
        Diag *d = diag_error((*pb)->span, "E0525", "shift count must be unsigned, found %s", type_str(r));
        (void)d;
        return false;
    }
    diag_error((*pb)->span, "E0412", "shift count must be an unsigned integer, found %s", type_str(r));
    return false;
}

static Type *check_binary(Expr *e, Type *expected) {
    int op = e->op;
    if (op == KW_AND || op == KW_OR) {
        check_expr_to(&e->a, t_bool);
        check_expr_to(&e->b, t_bool);
        return e->type = t_bool;
    }
    if (is_shift_op(op)) {
        Type *hint = expected && (expected->kind == TY_INT || ty_is_vec_int(expected) || expected->kind == TY_PARAM)
                         ? expected
                         : NULL;
        Type *l = check_expr(e->a, hint);
        bool ok = check_shift_count(&e->b);
        if (l->kind == TY_UNTYPED_INT) {
            if (e->a->is_const && e->b->is_const && e->b->type->kind == TY_INT) {
                i128 r;
                if (fold_int(op, e->a->cval, e->b->cval, NULL, &r, e->span)) {
                    e->cval = r;
                    e->is_const = true;
                }
                return e->type = t_untyped_int;
            }
            if (hint) {
                coerce(&e->a, hint);
                l = e->a->type;
            } else {
                finalize_untyped(&e->a);
                return e->type = t_invalid;
            }
        }
        if (l->kind != TY_INT && !ty_is_vec_int(l) && !param_has(l, C_INT) && l->kind != TY_INVALID) {
            diag_error(e->a->span, "E0513", "shift requires an integer operand, found %s", type_str(l));
            return e->type = t_invalid;
        }
        Type *k = e->b->type;
        if (ok && ty_is_vec_int(k) && l->kind != TY_INVALID &&
            !(ty_is_vec_int(l) && l->lanes == k->lanes && l->elem->bits == k->elem->bits)) {
            diag_error(e->b->span, "E0412", "per-lane shift counts must have the shape of the shifted vector: %s and %s",
                       type_str(l), type_str(k));
            return e->type = t_invalid;
        }
        if (ok && e->a->is_const && e->b->is_const && l->kind == TY_INT) {
            i128 r;
            if (fold_int(op, e->a->cval, e->b->cval, l, &r, e->span)) {
                e->cval = r;
                e->is_const = true;
            }
        }
        return e->type = l;
    }
    bool cmp = is_cmp_op(op);
    Type *hint = cmp ? NULL : expected;
    if (hint && (hint->kind == TY_ERRUNION)) hint = hint->elem;
    Type *l = check_expr(e->a, hint);
    Type *r = check_expr(e->b, ty_is_untyped(l) ? hint : l);
    Type *t;
    if (ty_is_untyped(l) && ty_is_untyped(r)) {
        t = (l->kind == TY_UNTYPED_FLOAT || r->kind == TY_UNTYPED_FLOAT) ? t_untyped_float : t_untyped_int;
        if (t == t_untyped_float) {
            if (l->kind == TY_UNTYPED_INT) e->a->fval = (double)e->a->cval;
            if (r->kind == TY_UNTYPED_INT) e->b->fval = (double)e->b->cval;
        }
        if (!cmp && hint && (hint->kind == TY_INT || hint->kind == TY_FLOAT || hint->kind == TY_PARAM)) {
            // e.g. `let x: u8 = 200 + 100` -> fold untyped and check at coercion
        }
    } else if (ty_is_untyped(l)) {
        coerce(&e->a, r);
        t = r;
    } else if (ty_is_untyped(r)) {
        coerce(&e->b, l);
        t = l;
    } else {
        t = l;
        if (l != r && l->kind != TY_INVALID && r->kind != TY_INVALID && l->kind != TY_NEVER && r->kind != TY_NEVER &&
            !(l->kind == TY_ERROR && r->kind == TY_ERROR)) {
            Diag *d = diag_error(e->span, "E0412", "operands of '%s' have different types: %s and %s",
                                 tok_kind_name((TokKind)op), type_str(l), type_str(r));
            diag_types(d, type_str(l), type_str(r));
            if (l->kind == TY_VEC && r == l->elem) {
                SourceFile *f = g_files.data[e->b->span.file];
                const char *txt = arena_strndup(f->text + e->b->span.start, e->b->span.end - e->b->span.start);
                DiagFix *fx = diag_fix(d, "broadcast the scalar to all lanes", "machine");
                diag_fix_edit(fx, e->b->span, arena_printf("@splat(%s)", txt));
            } else if (r->kind == TY_VEC && l == r->elem) {
                SourceFile *f = g_files.data[e->a->span.file];
                const char *txt = arena_strndup(f->text + e->a->span.start, e->a->span.end - e->a->span.start);
                DiagFix *fx = diag_fix(d, "broadcast the scalar to all lanes", "machine");
                diag_fix_edit(fx, e->a->span, arena_printf("@splat(%s)", txt));
            }
            return e->type = cmp ? t_bool : t_invalid;
        }
    }
    if (cmp) {
        check_cmp_op(op, t, e->span);
        if (ty_is_untyped(t) && e->a->is_const && e->b->is_const) {
            // two untyped constants: the comparison is a bool constant
            bool fl = t->kind == TY_UNTYPED_FLOAT;
            i128 a = e->a->cval, b = e->b->cval;
            double fa = e->a->fval, fb = e->b->fval;
            bool r = false;
            switch (op) {
            case TK_EQEQ: r = fl ? fa == fb : a == b; break;
            case TK_NE: r = fl ? fa != fb : a != b; break;
            case TK_LT: r = fl ? fa < fb : a < b; break;
            case TK_LE: r = fl ? fa <= fb : a <= b; break;
            case TK_GT: r = fl ? fa > fb : a > b; break;
            case TK_GE: r = fl ? fa >= fb : a >= b; break;
            }
            e->bval = r;
            e->is_const = true;
        }
        return e->type = t_bool;
    }
    if (!check_arith_op(op, t, e->span)) return e->type = t_invalid;
    // constant folding
    if (e->a->is_const && e->b->is_const) {
        if (t->kind == TY_UNTYPED_INT || t->kind == TY_INT) {
            i128 r2;
            if (fold_int(op, e->a->cval, e->b->cval, t->kind == TY_INT ? t : NULL, &r2, e->span)) {
                e->cval = r2;
                e->is_const = true;
            }
        } else if (t->kind == TY_UNTYPED_FLOAT || t->kind == TY_FLOAT) {
            double a = e->a->fval, b = e->b->fval, r2 = 0;
            switch (op) {
            case TK_PLUS: r2 = a + b; break;
            case TK_MINUS: r2 = a - b; break;
            case TK_STAR: r2 = a * b; break;
            case TK_SLASH: r2 = a / b; break;
            default: return e->type = t;
            }
            if (t->kind == TY_FLOAT && t->bits == 32) r2 = (double)(float)r2;
            e->fval = r2;
            e->is_const = true;
        }
    }
    return e->type = t;
}

static Type *check_unary(Expr *e, Type *expected) {
    int op = e->op;
    switch (op) {
    case KW_NOT:
        check_expr_to(&e->a, t_bool);
        return e->type = t_bool;
    case TK_AMP: {
        // the pointee type expected, if any, types an untyped temporary
        Type *hint = expected && expected->kind == TY_PTR ? expected->elem : NULL;
        check_operand(e->a, hint);
        check_value(e->a);
        if (ty_is_untyped(e->a->type)) {
            if (hint) coerce(&e->a, hint);
            else finalize_untyped(&e->a);
        }
        Type *t = e->a->type;
        if (t->kind == TY_INVALID) return e->type = t_invalid;
        if (!is_place(e->a)) {
            // a temporary: codegen gives it a slot that lives to the end of
            // the function (spec §7.5); only its immutable address is given
            if (e->bval) {
                diag_error(e->a->span, "E0703", "cannot take a mutable address of a temporary value");
                return e->type = t_invalid;
            }
            if (t->kind == TY_VOID) {
                diag_error(e->a->span, "E0703", "cannot take the address of a void value");
                return e->type = t_invalid;
            }
        } else {
            if (e->bval && !is_mut_place(e->a)) not_mutable(e->a, "take a mutable address");
            mark_addr_taken(e->a);
        }
        return e->type = type_ptr(t, e->bval);
    }
    case TK_MINUS: {
        Type *t = check_expr(e->a, expected && expected->kind != TY_ERRUNION ? expected : NULL);
        if (ty_is_untyped(t)) {
            if (e->a->is_const) {
                e->cval = -e->a->cval;
                e->fval = -e->a->fval;
                e->is_const = true;
            }
            return e->type = t;
        }
        if ((t->kind == TY_INT && !t->is_signed) || (ty_is_vec_int(t) && !t->elem->is_signed)) {
            Diag *d = diag_error(e->span, "E0520", "unary '-' cannot be applied to unsigned type %s", type_str(t));
            DiagFix *fx = diag_fix(d, "use wrapping negation", "suggestion");
            diag_fix_edit(fx, (Span){e->span.file, e->span.start, e->span.start + 1}, "-%");
            return e->type = t_invalid;
        }
        if ((t->kind == TY_INT) || t->kind == TY_FLOAT || ty_is_vec_float(t) || param_has(t, C_SIGNED) ||
            param_has(t, C_FLOAT) || param_has(t, C_FLOAT_VEC) || t->kind == TY_INVALID) {
            if (e->a->is_const && t->kind == TY_INT) {
                if (!fits_type(-e->a->cval, t)) diag_error(e->span, "E0530", "overflow in constant expression");
                e->cval = -e->a->cval;
                e->is_const = true;
            } else if (e->a->is_const && t->kind == TY_FLOAT) {
                e->fval = -e->a->fval;
                e->is_const = true;
            }
            return e->type = t;
        }
        if (ty_is_vec_int(t)) return e->type = t;  // lane-wise, wrapping (§13.1)
        diag_error(e->span, "E0513", "unary '-' cannot be applied to %s", type_str(t));
        return e->type = t_invalid;
    }
    case TK_MINUS_PCT:
    case TK_TILDE: {
        Type *t = check_expr(e->a, expected);
        if (ty_is_untyped(t)) {
            if (expected && (expected->kind == TY_INT || expected->kind == TY_PARAM)) {
                coerce(&e->a, expected);
                t = e->a->type;
            } else {
                finalize_untyped(&e->a);
                return e->type = t_invalid;
            }
        }
        bool ok = t->kind == TY_INT || ty_is_vec_int(t) || param_has(t, C_INT) || t->kind == TY_INVALID ||
                  (op == TK_TILDE && t->kind == TY_V128);
        if (!ok) {
            diag_error(e->span, "E0513", "operator '%s' cannot be applied to %s", tok_kind_name((TokKind)op), type_str(t));
            return e->type = t_invalid;
        }
        if (e->a->is_const && t->kind == TY_INT) {
            e->cval = op == TK_TILDE ? wrap_to(~e->a->cval, t) : wrap_to(-e->a->cval, t);
            e->is_const = true;
        }
        return e->type = t;
    }
    }
    return e->type = t_invalid;
}

// ---------------------------------------------------------------------------
// Names
// ---------------------------------------------------------------------------
static void decl_ref(Expr *e, Decl *d) {
    resolve_decl(d);
    e->decl = d;
    switch (d->kind) {
    case D_FN:
    case D_HOST_FN:
        e->cat = CAT_FUNC;
        e->ngargs = 0;
        e->type = NULL;
        return;
    case D_CONST:
        e->cat = CAT_VALUE;
        e->type = d->type;
        if (d->cexpr) {
            e->is_const = d->cexpr->is_const || d->cexpr->kind == E_BOOL;
            e->cval = d->cexpr->cval;
            e->fval = d->cexpr->fval;
            e->bval = d->cexpr->bval;
            if (d->type->kind == TY_FLOAT && d->cexpr->type->kind == TY_FLOAT) e->fval = d->cexpr->fval;
        }
        return;
    case D_GLOBAL:
        e->cat = CAT_VALUE;
        e->type = d->type;
        if (C && C->const_ctx) e->is_const = false;
        return;
    case D_STRUCT:
    case D_UNION:
    case D_ENUM:
        if (d->gparams.len) {
            e->cat = CAT_GENERIC;
        } else {
            e->cat = CAT_TYPE;
            e->tval = d->type;
        }
        return;
    default:
        e->cat = CAT_VALUE;
        e->type = t_invalid;
        return;
    }
}

static void resolve_ident(Expr *e) {
    Local *l = scope_lookup(e->name);
    if (l) {
        e->local = l;
        e->cat = CAT_VALUE;
        e->type = l->type;
        return;
    }
    Type *gp = lookup_gparam(&C->tc, e->name);
    if (gp) {
        if (gp->is_const) {
            e->cat = CAT_VALUE;
            e->type = gp->const_type;
            e->tval = gp;
        } else {
            e->cat = CAT_TYPE;
            e->tval = gp;
        }
        return;
    }
    Decl *d = module_lookup(C->tc.mod, e->name);
    if (d) {
        decl_ref(e, d);
        return;
    }
    Decl *imp = symmap_get(&C->tc.mod->imports, e->name);
    if (imp) {
        e->cat = CAT_MODULE;
        e->module = imp->imported;
        e->type = t_invalid;
        if (!imp->imported) e->cat = CAT_VALUE;
        return;
    }
    diag_error(e->span, "E0202", "unknown name '%s'", e->name);
    e->cat = CAT_VALUE;
    e->type = t_invalid;
}

// ---------------------------------------------------------------------------
// Field access
// ---------------------------------------------------------------------------
// A method or associated function is visible outside its module only if it is
// `pub` (spec §4.2).
static void check_method_visible(Expr *e, Decl *m) {
    if (!m->is_pub && m->module != C->tc.mod) {
        Diag *dg = diag_error(e->name_span, "E0203", "'%s' is not public", e->name);
        diag_note(dg, m->name_span, "declared here");
    }
}

static Type *check_field(Expr *e) {
    Expr *a = e->a;
    check_operand(a, NULL);
    Sym name = e->name;
    switch (a->cat) {
    case CAT_MODULE: {
        Decl *d = module_lookup(a->module, name);
        if (!d) {
            diag_error(e->name_span, "E0202", "module has no declaration '%s'", name);
            e->type = t_invalid;
            return e->type;
        }
        if (!d->is_pub) {
            Diag *dg = diag_error(e->name_span, "E0203", "'%s' is not public", name);
            diag_note(dg, d->name_span, "declared here");
        }
        decl_ref(e, d);
        e->op = d->kind == D_CONST ? FK_CONST : FK_DECL;
        return e->type;
    }
    case CAT_GENERIC: {
        Diag *d = diag_error(a->span, "E0460", "generic type '%s' must be given arguments: %s.[...]", a->decl->name,
                             a->decl->name);
        (void)d;
        e->type = t_invalid;
        return e->type;
    }
    case CAT_TYPE: {
        Type *t = a->tval;
        if (t->kind != TY_STRUCT && t->kind != TY_UNION && t->kind != TY_ENUM) {
            diag_error(e->name_span, "E0404", "type %s has no member '%s'", type_str(t), name);
            e->type = t_invalid;
            return e->type;
        }
        Decl *m = find_method(t->decl, name);
        if (m) {
            check_method_visible(e, m);
            resolve_decl(m);
            e->cat = CAT_FUNC;
            e->op = FK_DECL;
            e->decl = m;
            e->gargs = t->args;
            e->ngargs = t->nargs;
            return NULL;
        }
        if (t->kind == TY_UNION) {
            int v = find_variant(t->decl, name);
            if (v >= 0) {
                e->index = v;
                e->tval = t;
                if (t->decl->variants.data[v].type) {
                    e->cat = CAT_VARIANT;
                    return NULL;
                }
                e->cat = CAT_VALUE;
                e->op = FK_VARIANT_UNIT;
                return e->type = t;
            }
        }
        if (t->kind == TY_ENUM) {
            int v = find_variant(t->decl, name);
            if (v >= 0) {
                e->index = v;
                e->cat = CAT_VALUE;
                e->op = FK_CONST;
                e->is_const = true;
                e->cval = t->decl->variants.data[v].value;
                return e->type = t;
            }
            diag_error(e->name_span, "E0404", "enum %s has no member or method '%s'", type_str(t), name);
            e->type = t_invalid;
            return e->type;
        }
        diag_error(e->name_span, "E0404", "type %s has no method or variant '%s'", type_str(t), name);
        e->type = t_invalid;
        return e->type;
    }
    case CAT_FUNC:
    case CAT_METHOD:
    case CAT_VARIANT:
        check_value(a);
        break;
    default: break;
    }
    Type *t = a->type;
    if (!t || t->kind == TY_INVALID) return e->type = t_invalid;
    Type *base = t;
    if (t->kind == TY_PTR && (t->elem->kind == TY_STRUCT || t->elem->kind == TY_UNION || t->elem->kind == TY_ENUM)) {
        base = t->elem;
        e->self_mode = SELF_DEREF;
    }
    if (base->kind == TY_STRUCT) {
        int fi = find_field(base->decl, name);
        if (fi >= 0) {
            e->op = FK_STRUCT;
            e->index = fi;
            return e->type = type_field(base, fi);
        }
    }
    if (base->kind == TY_STRUCT || base->kind == TY_UNION || base->kind == TY_ENUM) {
        Decl *m = find_method(base->decl, name);
        if (m) {
            check_method_visible(e, m);
            resolve_decl(m);
            e->cat = CAT_METHOD;
            e->decl = m;
            e->b = a;  // receiver
            e->gargs = base->args;
            e->ngargs = base->nargs;
            return NULL;
        }
    }
    if (t->kind == TY_SLICE) {
        if (name == intern_cstr("len")) {
            e->op = FK_SLICE_LEN;
            return e->type = t_usize;
        }
        if (name == intern_cstr("ptr")) {
            e->op = FK_SLICE_PTR;
            return e->type = type_ptr(t->elem, t->mut);
        }
    }
    if (t->kind == TY_ARRAY && name == intern_cstr("len")) {
        e->op = FK_ARRAY_LEN;
        if (!t->len_param) {
            e->is_const = true;
            e->cval = t->len;
        }
        return e->type = t_usize;
    }
    diag_error(e->name_span, "E0404", "%s has no field or method '%s'", type_str(t), name);
    return e->type = t_invalid;
}

// ---------------------------------------------------------------------------
// Generic application  x.[args]
// ---------------------------------------------------------------------------
static Type *resolve_garg(TypeExpr *te) {
    if (te->kind == TE_INTLIT) return type_constint((int64_t)te->ival);
    if (te->kind == TE_NAMED && !te->has_args && !te->qual) {
        Type *gp = lookup_gparam(&C->tc, te->name);
        if (gp) return gp;
        Decl *d = module_lookup(C->tc.mod, te->name);
        if (d && d->kind == D_CONST) {
            resolve_decl(d);
            if (d->cexpr && d->cexpr->is_const && d->type->kind == TY_INT) return type_constint((int64_t)d->cexpr->cval);
        }
    }
    return resolve_type(te, &C->tc);
}

static Type *check_apply(Expr *e) {
    Expr *a = e->a;
    check_operand(a, NULL);
    int n = e->targs.len;
    Type **args = NEWN(Type *, n + 1);
    for (int i = 0; i < n; i++) args[i] = resolve_garg(e->targs.data[i]);
    if (a->cat == CAT_GENERIC) {
        Decl *d = a->decl;
        if (!check_generic_args(d, args, n, e->span, &e->targs)) {
            e->cat = CAT_TYPE;
            e->tval = t_invalid;
            return NULL;
        }
        e->cat = CAT_TYPE;
        e->tval = type_named(d, args, n);
        return NULL;
    }
    if (a->cat == CAT_FUNC || a->cat == CAT_METHOD) {
        Decl *fn = a->decl;
        if (fn->kind == D_HOST_FN || fn->gparams.len == 0) {
            diag_error(e->span, "E0468", "'%s' is not generic", fn->name);
            e->cat = a->cat;
            e->decl = fn;
            e->gargs = a->gargs;
            e->ngargs = a->ngargs;
            e->b = a->b;
            e->self_mode = a->self_mode;
            return NULL;
        }
        if (a->ngargs != fn->nall_gparams - fn->gparams.len) {
            diag_error(e->span, "E0469", "generic arguments were already given");
        }
        check_generic_args(fn, args, n, e->span, &e->targs);
        int nowner = fn->nall_gparams - fn->gparams.len;
        e->cat = a->cat;
        e->decl = fn;
        e->ngargs = fn->nall_gparams;
        e->gargs = NEWN(Type *, fn->nall_gparams + 1);
        for (int i = 0; i < nowner; i++) e->gargs[i] = a->gargs[i];
        for (int i = 0; i < fn->gparams.len; i++) e->gargs[nowner + i] = i < n ? args[i] : t_invalid;
        e->b = a->b;
        e->self_mode = a->self_mode;
        return NULL;
    }
    if (a->cat == CAT_TYPE) {
        diag_error(e->span, "E0468", "type %s is not generic", type_str(a->tval));
    } else if (a->type && a->type->kind != TY_INVALID) {
        diag_error(e->span, "E0468", "generic arguments can only be applied to generic types and functions");
    }
    e->cat = CAT_VALUE;
    e->type = t_invalid;
    return NULL;
}

// ---------------------------------------------------------------------------
// Calls
// ---------------------------------------------------------------------------
static bool unify(Type *pat, Type *act, Type **vars, Type **sol, int nv) {
    if (!pat->has_params) return true;
    if (!act || act->kind == TY_INVALID || ty_is_untyped(act) || act->kind == TY_NEVER) return true;
    switch (pat->kind) {
    case TY_PARAM:
        for (int i = 0; i < nv; i++) {
            if (vars[i] != pat) continue;
            if (!sol[i]) {
                sol[i] = act;
                return true;
            }
            return sol[i] == act;
        }
        return true;
    case TY_PTR:
    case TY_SLICE:
        if (act->kind != pat->kind) return true;
        return unify(pat->elem, act->elem, vars, sol, nv);
    case TY_ERRUNION:
        if (act->kind == TY_ERRUNION) return unify(pat->elem, act->elem, vars, sol, nv);
        return unify(pat->elem, act, vars, sol, nv);
    case TY_ARRAY:
        if (act->kind != TY_ARRAY) return true;
        if (!unify(pat->elem, act->elem, vars, sol, nv)) return false;
        if (pat->len_param) {
            Type *lv = act->len_param ? act->len_param : type_constint(act->len);
            return unify(pat->len_param, lv, vars, sol, nv);
        }
        return true;
    case TY_FN:
        if (act->kind != TY_FN || act->nparams != pat->nparams) return true;
        for (int i = 0; i < pat->nparams; i++)
            if (!unify(pat->params[i], act->params[i], vars, sol, nv)) return false;
        return unify(pat->ret, act->ret, vars, sol, nv);
    case TY_STRUCT:
    case TY_UNION:
        if (act->decl != pat->decl) return true;
        for (int i = 0; i < pat->nargs; i++)
            if (!unify(pat->args[i], act->args[i], vars, sol, nv)) return false;
        return true;
    default: return true;
    }
}

static bool mentions(Type *t, Type **vars, int nv) {
    if (!t->has_params) return false;
    for (int i = 0; i < nv; i++) {
        Type *from[1] = {vars[i]};
        Type *to[1] = {t_invalid};
        if (type_subst(t, from, to, 1) != t) return true;
    }
    return false;
}

// Arguments in the variadic part of a C call undergo C's default argument
// promotions, so only already-promoted types are accepted (spec §14.5).
static void check_vararg(Expr **pe) {
    Type *t = check_expr(*pe, NULL);
    if (t->kind == TY_UNTYPED_INT) {
        coerce(pe, fits_type((*pe)->cval, t_i32) ? t_i32 : t_i64);
        return;
    }
    if (t->kind == TY_UNTYPED_FLOAT) {
        coerce(pe, t_f64);
        return;
    }
    bool ok = (t->kind == TY_INT && t->bits >= 32) || (t->kind == TY_FLOAT && t->bits == 64) || t->kind == TY_PTR ||
              t->kind == TY_FN || t->kind == TY_INVALID;
    if (ok) return;
    Diag *d = diag_error((*pe)->span, "E0824",
                         "C variadic arguments must be i32, u32, i64, u64, isize, usize, f64 or pointers "
                         "(C's default argument promotions); found %s",
                         type_str(t));
    if (t->kind == TY_INT || t->kind == TY_FLOAT) {
        SourceFile *f = g_files.data[(*pe)->span.file];
        const char *txt = arena_strndup(f->text + (*pe)->span.start, (*pe)->span.end - (*pe)->span.start);
        const char *to = t->kind == TY_FLOAT ? "f64" : (t->kind == TY_INT && t->is_signed) ? "i32" : "u32";
        DiagFix *fx = diag_fix(d, "apply the promotion explicitly", "machine");
        diag_fix_edit(fx, (*pe)->span, arena_printf("@as(%s, %s)", to, txt));
    }
}

static void check_args_count(Expr *e, int want, Decl *fn) {
    if (fn && fn->variadic) {
        if (e->args.len < want) {
            Diag *d = diag_error(e->span, "E0420", "expected at least %d argument%s, found %d", want, want == 1 ? "" : "s",
                                 e->args.len);
            diag_note(d, fn->name_span, "'%s' declared here", fn->name);
        }
        return;
    }
    if (e->args.len != want) {
        Diag *d = diag_error(e->span, "E0420", "expected %d argument%s, found %d", want, want == 1 ? "" : "s",
                             e->args.len);
        if (fn) diag_note(d, fn->name_span, "'%s' declared here", fn->name);
    }
}

static Type *check_call(Expr *e, Type *expected) {
    Expr *callee = e->a;
    check_operand(callee, NULL);
    e->cat = CAT_VALUE;
    switch (callee->cat) {
    case CAT_FUNC:
    case CAT_METHOD: {
        Decl *fn = callee->decl;
        resolve_decl(fn);
        bool method = callee->cat == CAT_METHOD;
        if (method && fn->self_kind == SELF_NONE) {
            diag_error(callee->span, "E0421", "'%s' has no 'self' parameter; call it as %s.%s(...)", fn->name,
                       fn->owner->name, fn->name);
            for (int i = 0; i < e->args.len; i++) check_expr(e->args.data[i], NULL);
            return e->type = t_invalid;
        }
        int first = method ? 1 : 0;
        int nparams = fn->params.len - first;
        check_args_count(e, nparams, fn);
        int nall = fn->nall_gparams;
        int nown = fn->kind == D_HOST_FN ? 0 : fn->gparams.len;
        int nowner = nall - nown;
        Type **gargs = NEWN(Type *, nall + 1);
        for (int i = 0; i < callee->ngargs && i < nall; i++) gargs[i] = callee->gargs[i];
        bool deduce = nown > 0 && callee->ngargs < nall;
        Type **owner_from = fn->all_gparams, **owner_to = gargs;
        Type **own_vars = fn->all_gparams ? fn->all_gparams + nowner : NULL;
        // phase 1: check args
        int nargs = e->args.len < nparams ? e->args.len : nparams;
        for (int i = 0; i < e->args.len; i++) {
            if (i >= nparams) {
                if (fn->variadic) check_vararg(&e->args.data[i]);
                else check_expr(e->args.data[i], NULL);
                continue;
            }
            Type *pt = type_subst(fn->param_types[first + i], owner_from, owner_to, nowner);
            if (deduce && mentions(pt, own_vars, nown)) check_expr(e->args.data[i], NULL);
            else {
                if (!deduce) pt = type_subst(fn->param_types[first + i], fn->all_gparams, gargs, nall);
                check_expr(e->args.data[i], pt);
            }
        }
        // phase 2: deduce
        if (deduce) {
            Type **sol = NEWN(Type *, nown + 1);
            bool ok = true;
            for (int i = 0; i < nargs; i++) {
                Type *pt = type_subst(fn->param_types[first + i], owner_from, owner_to, nowner);
                Expr *arg = e->args.data[i];
                Type *before[16] = {0};
                for (int k = 0; k < nown && k < 16; k++) before[k] = sol[k];
                if (!unify(pt, arg->type, own_vars, sol, nown)) {
                    for (int k = 0; k < nown && k < 16; k++) {
                        if (before[k] && sol[k] == before[k]) {
                            Type *tmp[1] = {NULL};
                            unify(pt, arg->type, own_vars + k, tmp, 1);
                            if (tmp[0] && tmp[0] != before[k]) {
                                Diag *d = diag_error(arg->span, "E0464",
                                                     "conflicting types for generic parameter '%s': %s and %s",
                                                     fn->gparams.data[k].name, type_str(before[k]), type_str(tmp[0]));
                                diag_note(d, fn->gparams.data[k].span, "'%s' declared here", fn->gparams.data[k].name);
                            }
                        }
                    }
                    ok = false;
                }
            }
            for (int k = 0; k < nown && ok; k++) {
                if (!sol[k]) {
                    Diag *d = diag_error(e->span, "E0463",
                                         "cannot deduce generic parameter '%s' of '%s'; give it explicitly: %s.[...](...)",
                                         fn->gparams.data[k].name, fn->name, fn->name);
                    diag_note(d, fn->gparams.data[k].span, "'%s' declared here", fn->gparams.data[k].name);
                    ok = false;
                }
            }
            for (int k = 0; k < nown; k++) gargs[nowner + k] = sol[k] ? sol[k] : t_invalid;
            if (ok) check_generic_args(fn, gargs + nowner, nown, e->span, NULL);
        }
        // phase 3: coerce args
        for (int i = 0; i < nargs; i++) {
            Type *pt = type_subst(fn->param_types[first + i], fn->all_gparams, gargs, nall);
            coerce(&e->args.data[i], pt);
        }
        e->decl = fn;
        e->gargs = gargs;
        e->ngargs = nall;
        e->op = CALLK_DIRECT;
        if (fn->is_inline && C->fn && C->fn->is_inline) {
            vec_push(C->fn->inline_callees, fn);
            vec_push(C->fn->inline_call_spans, e->span);
        }
        // receiver
        if (method) {
            Expr *recv = callee->b;
            Type *rt = recv->type;
            bool via_ptr = callee->self_mode == SELF_DEREF;
            switch (fn->self_kind) {
            case SELF_VALUE: e->self_mode = via_ptr ? SELF_DEREF : SELF_VALUE; break;
            case SELF_ADDR:
                if (via_ptr) e->self_mode = SELF_PASS;
                else {
                    // a temporary receiver gets a slot, as &temporary does
                    e->self_mode = SELF_ADDR;
                    mark_addr_taken(recv);
                }
                break;
            case SELF_PASS:
                if (via_ptr) {
                    if (!rt->mut) {
                        Diag *d = diag_error(recv->span, "E0702", "method '%s' takes '*mut Self', but the receiver is %s",
                                             fn->name, type_str(rt));
                        (void)d;
                    }
                    e->self_mode = SELF_PASS;
                } else if (!is_place(recv)) {
                    diag_error(recv->span, "E0703",
                               "method '%s' needs the mutable address of its receiver, but it is a temporary", fn->name);
                } else {
                    if (!is_mut_place(recv)) not_mutable(recv, arena_printf("call '%s' (it takes *mut Self)", fn->name));
                    e->self_mode = SELF_ADDR;
                    mark_addr_taken(recv);
                }
                break;
            default: break;
            }
            e->b = recv;
        }
        (void)expected;
        return e->type = type_subst(fn->ret_type, fn->all_gparams, gargs, nall);
    }
    case CAT_VARIANT: {
        Type *ut = callee->tval;
        int v = callee->index;
        Type *pt = type_variant(ut, v);
        check_args_count(e, 1, NULL);
        for (int i = 0; i < e->args.len; i++) {
            if (i == 0) check_expr_to(&e->args.data[0], pt);
            else check_expr(e->args.data[i], NULL);
        }
        e->op = CALLK_VARIANT;
        e->index = v;
        e->tval = ut;
        return e->type = ut;
    }
    case CAT_TYPE:
        diag_error(callee->span, "E0406", "%s is a type, not a function; use %s.{ ... } to construct a struct",
                   type_str(callee->tval), type_str(callee->tval));
        return e->type = t_invalid;
    case CAT_GENERIC:
        diag_error(callee->span, "E0460", "generic type '%s' must be given arguments", callee->decl->name);
        return e->type = t_invalid;
    case CAT_MODULE:
        diag_error(callee->span, "E0406", "a module cannot be called");
        return e->type = t_invalid;
    default: break;
    }
    Type *ft = callee->type;
    if (!ft || ft->kind == TY_INVALID) {
        for (int i = 0; i < e->args.len; i++) check_expr(e->args.data[i], NULL);
        return e->type = t_invalid;
    }
    if (ft->kind != TY_FN) {
        diag_error(callee->span, "E0406", "cannot call a value of type %s", type_str(ft));
        for (int i = 0; i < e->args.len; i++) check_expr(e->args.data[i], NULL);
        return e->type = t_invalid;
    }
    check_args_count(e, ft->nparams, NULL);
    for (int i = 0; i < e->args.len; i++) {
        if (i < ft->nparams) check_expr_to(&e->args.data[i], ft->params[i]);
        else check_expr(e->args.data[i], NULL);
    }
    e->op = CALLK_INDIRECT;
    return e->type = ft->ret;
}

// ---------------------------------------------------------------------------
// Literals
// ---------------------------------------------------------------------------
static Type *check_struct_lit(Expr *e, Type *expected) {
    Type *t = NULL;
    if (e->a) {
        check_operand(e->a, NULL);
        if (e->a->cat == CAT_TYPE && e->a->tval->kind == TY_STRUCT) t = e->a->tval;
        else if (e->a->cat == CAT_GENERIC) {
            diag_error(e->a->span, "E0460", "generic type '%s' must be given arguments: %s.[...]", e->a->decl->name,
                       e->a->decl->name);
        } else if (!(e->a->cat == CAT_TYPE && e->a->tval->kind == TY_INVALID) &&
                   !(e->a->type && e->a->type->kind == TY_INVALID)) {
            diag_error(e->a->span, "E0408", "struct literal requires a struct type");
        }
    } else if (expected) {
        Type *x = expected->kind == TY_ERRUNION ? expected->elem : expected;
        if (x->kind == TY_STRUCT) t = x;
        else if (x->kind != TY_INVALID)
            diag_error(e->span, "E0408", "anonymous struct literal where %s is expected", type_str(expected));
    } else {
        diag_error(e->span, "E0503", "cannot infer the type of '.{ ... }'; write T.{ ... }");
    }
    if (!t) {
        for (int i = 0; i < e->inits.len; i++) check_expr(e->inits.data[i].value, NULL);
        return e->type = t_invalid;
    }
    Decl *d = t->decl;
    int nf = d->fields.len;
    bool order_ok = e->inits.len == nf;
    for (int i = 0; i < e->inits.len && order_ok; i++)
        if (e->inits.data[i].name != d->fields.data[i].name) order_ok = false;
    if (!order_ok) {
        // Build a helpful message: expected field order.
        CharVec b = {0};
        for (int i = 0; i < nf; i++) {
            const char *n = d->fields.data[i].name;
            if (i) vec_push(b, ',');
            if (i) vec_push(b, ' ');
            while (*n) vec_push(b, *n++);
        }
        vec_push(b, 0);
        Diag *dg = diag_error(e->span, "E0410", "struct literal of %s must list every field, in declaration order: %s",
                              type_str(t), b.data);
        free(b.data);
        // machine fix: reorder when the set of names is exactly right
        bool same_set = e->inits.len == nf;
        for (int i = 0; i < nf && same_set; i++) {
            bool found = false;
            for (int j = 0; j < e->inits.len; j++)
                if (e->inits.data[j].name == d->fields.data[i].name) found = true;
            if (!found) same_set = false;
        }
        if (same_set && nf > 0) {
            SourceFile *f = g_files.data[e->span.file];
            CharVec r = {0};
            for (int i = 0; i < nf; i++) {
                for (int j = 0; j < e->inits.len; j++) {
                    FieldInit *fi = &e->inits.data[j];
                    if (fi->name != d->fields.data[i].name) continue;
                    Span vs = fi->value->span;
                    const char *nm = fi->name;
                    if (i) {
                        vec_push(r, ',');
                        vec_push(r, ' ');
                    }
                    while (*nm) vec_push(r, *nm++);
                    vec_push(r, ':');
                    vec_push(r, ' ');
                    for (uint32_t k = vs.start; k < vs.end; k++) vec_push(r, f->text[k]);
                }
            }
            vec_push(r, 0);
            Span whole = {e->span.file, e->inits.data[0].span.start,
                          e->inits.data[e->inits.len - 1].value->span.end};
            DiagFix *fx = diag_fix(dg, "reorder the fields", "machine");
            diag_fix_edit(fx, whole, arena_strndup(r.data, strlen(r.data)));
            free(r.data);
        }
    }
    for (int i = 0; i < e->inits.len; i++) {
        FieldInit *fi = &e->inits.data[i];
        int idx = find_field(d, fi->name);
        if (idx < 0) {
            diag_error(fi->span, "E0404", "%s has no field '%s'", type_str(t), fi->name);
            check_expr(fi->value, NULL);
            continue;
        }
        check_expr_to(&fi->value, type_field(t, idx));
    }
    e->tval = t;
    return e->type = t;
}

static Type *check_array_lit(Expr *e, Type *expected) {
    Type *x = expected && expected->kind == TY_ERRUNION ? expected->elem : expected;
    Type *elem = x && x->kind == TY_ARRAY ? x->elem : NULL;
    if (e->kind == E_ARRAY_REPEAT) {
        check_expr_to(&e->b, t_usize);
        // a const generic parameter gives an array of that (symbolic) length
        Type *lenp = e->b->kind == E_IDENT && e->b->tval && e->b->tval->kind == TY_PARAM && e->b->tval->is_const
                         ? e->b->tval : NULL;
        if (!lenp && !e->b->is_const && e->b->type->kind != TY_INVALID) {
            diag_error(e->b->span, "E0350", "array repeat count must be a constant");
            return e->type = t_invalid;
        }
        int64_t n = (int64_t)e->b->cval;
        if (elem) check_expr_to(&e->a, elem);
        else {
            elem = check_expr(e->a, NULL);
            if (ty_is_untyped(elem)) {
                finalize_untyped(&e->a);
                return e->type = t_invalid;
            }
        }
        if (lenp) return e->type = type_array_param(elem, lenp);
        Type *t = type_array(elem, n);
        if (x && x->kind == TY_ARRAY && !x->len_param && x->len != n && x->elem->kind != TY_INVALID)
            type_mismatch(e, x, t);
        return e->type = t;
    }
    int n = e->args.len;
    for (int i = 0; i < n; i++) {
        if (elem) check_expr_to(&e->args.data[i], elem);
        else {
            elem = check_expr(e->args.data[i], NULL);
            if (ty_is_untyped(elem)) {
                finalize_untyped(&e->args.data[i]);
                elem = NULL;
            }
        }
    }
    if (!elem) return e->type = t_invalid;
    Type *t = type_array(elem, n);
    if (x && x->kind == TY_ARRAY && !x->len_param && x->len != n) type_mismatch(e, x, t);
    return e->type = t;
}

// ---------------------------------------------------------------------------
// catch / try
// ---------------------------------------------------------------------------
static bool check_leave_ok(Span sp, bool is_loop_exit) {
    if (C->defer_depth > 0 && (!is_loop_exit || C->loop_depth <= C->defer_loop_base)) {
        diag_error(sp, "E0640", "control flow cannot leave a deferred statement");
        return false;
    }
    return true;
}

// The local whose storage place e is part of (NULL: a global, or memory
// reached through a pointer or a slice).
static Local *place_local(Expr *e) {
    if (e->kind == E_IDENT) return e->local;
    if (e->kind == E_FIELD && e->op == FK_STRUCT && e->self_mode != SELF_DEREF) return place_local(e->a);
    if (e->kind == E_INDEX && e->op == IDX_ARRAY) return place_local(e->a);
    return NULL;
}

// A returned address of a local (`&x`, `&mut x.f`, `&a[i]`) or slice of a
// local array (`a[..]`, `a[i..j]`) dangles once the function returns (spec
// §12.2: W0710).
static void check_escape(Expr *e0) {
    Expr *e = e0;
    while (e->kind == E_CONV) e = e->a;
    if (!e->type || e->type->kind == TY_INVALID) return;
    Local *l = NULL;
    if (e->kind == E_UNARY && e->op == TK_AMP && is_place(e->a)) l = place_local(e->a);
    else if (e->kind == E_SLICE && e->op == IDX_ARRAY) l = place_local(e->a);
    if (l) diag_warning(e0->span, "W0710", "this points into local '%s', whose storage ends when the function returns", l->name);
}

static void check_return_value(Expr **pe, Span sp) {
    if (!C->fn) {
        diag_error(sp, "E0602", "'return' outside of a function");
        return;
    }
    check_leave_ok(sp, false);
    Type *r = C->ret;
    if (!pe || !*pe) {
        if (r->kind != TY_VOID && !(r->kind == TY_ERRUNION && r->elem->kind == TY_VOID) && r->kind != TY_INVALID)
            diag_error(sp, "E0602", "this function must return a value of type %s", type_str(r));
        return;
    }
    if (r->kind == TY_VOID) {
        check_expr(*pe, NULL);
        diag_error((*pe)->span, "E0603", "a void function cannot return a value");
        return;
    }
    if (r->kind == TY_ERRUNION && r->elem->kind == TY_VOID) {
        check_expr_to(pe, r);
        return;
    }
    check_expr_to(pe, r);
    check_escape(*pe);
}

void check_return_value_ext(Expr **pe, Span sp) { check_return_value(pe, sp); }

static Type *check_catch(Expr *e) {
    Type *t = check_expr(e->a, NULL);
    Type *vt = t_invalid;
    Type *et = t_error;
    if (t->kind == TY_ERRUNION) {
        vt = t->elem;
        et = type_errset(t->decl);
    } else if (t->kind != TY_INVALID) diag_error(e->a->span, "E0611", "'catch' requires an error union (!T), found %s", type_str(t));
    scope_push();
    if (e->capture) e->capture_local = declare_local(e->capture, e->capture_span, et, false, false);
    switch (e->hkind) {
    case H_VALUE:
        if (vt->kind == TY_VOID) {
            // For !void only a diverging handler (e.g. @trap()) makes sense.
            Type *ht = check_expr(e->hexpr, NULL);
            if (ht->kind != TY_NEVER && ht->kind != TY_INVALID)
                diag_error(e->hexpr->span, "E0613", "a fallback value makes no sense for !void; use a block");
        } else {
            check_expr_to(&e->hexpr, vt);
        }
        break;
    case H_RETURN: check_return_value(e->hexpr ? &e->hexpr : NULL, e->span); break;
    case H_BREAK:
    case H_CONTINUE: {
        // the label, if any, is in e->name; the loops left in e->index
        int d = check_jump_target(e->name, e->name ? e->name_span : e->span, e->hkind == H_BREAK);
        e->index = d < 0 ? 0 : d;
        break;
    }
    case H_BLOCK:
        // For !void the block may complete normally: execution continues after
        // the expression. For !T there is no value to produce, so it must diverge.
        if (!check_block(e->hblock) && vt->kind != TY_VOID && vt->kind != TY_INVALID)
            diag_error(e->hblock->span, "E0612",
                       "this 'catch' block must not complete normally, because the expression needs a value of type %s "
                       "(end it with return, break, continue or @trap(), or use 'catch <value>')",
                       type_str(vt));
        break;
    }
    scope_pop();
    return e->type = vt;
}

static Type *check_try(Expr *e) {
    Type *t = check_expr(e->a, NULL);
    if (t->kind == TY_INVALID) return e->type = t_invalid;
    if (t->kind != TY_ERRUNION) {
        diag_error(e->span, "E0611", "'!' requires an error union (!T), found %s", type_str(t));
        return e->type = t_invalid;
    }
    if (!C->fn || !C->ret || C->ret->kind != TY_ERRUNION) {
        Diag *d = diag_error(e->span, "E0610", "'!' can only be used in a function that returns an error union");
        if (C->fn && C->fn->ret_te) {
            DiagFix *fx = diag_fix(d, "make the function return an error union", "suggestion");
            Span rs = C->fn->ret_te->span;
            diag_fix_edit(fx, (Span){rs.file, rs.start, rs.start}, "!");
        }
    } else {
        check_leave_ok(e->span, false);
        try_fits(e->span, t->decl, C->ret->decl);
    }
    return e->type = t->elem;
}

// ---------------------------------------------------------------------------
// Index / slice
// ---------------------------------------------------------------------------
static Type *check_index(Expr *e) {
    Type *t = check_expr(e->a, NULL);
    Type *elem = NULL;
    if (t->kind == TY_ARRAY) {
        elem = t->elem;
        e->op = IDX_ARRAY;
    } else if (t->kind == TY_SLICE) {
        elem = t->elem;
        e->op = IDX_SLICE;
    } else if (t->kind == TY_PTR && t->elem->kind == TY_ARRAY) {
        elem = t->elem->elem;
        e->op = IDX_PTR_ARRAY;
    } else if (t->kind != TY_INVALID) {
        diag_error(e->a->span, "E0407", "cannot index a value of type %s", type_str(t));
    }
    if (e->kind == E_INDEX) {
        Type *it = check_expr(e->b, t_usize);
        if (it->kind == TY_UNTYPED_INT) coerce(&e->b, t_usize);
        else if (it != t_usize && it->kind != TY_INVALID) {
            Diag *d = diag_error(e->b->span, "E0540", "index must have type usize, found %s", type_str(it));
            if (it->kind == TY_INT) {
                SourceFile *f = g_files.data[e->b->span.file];
                const char *txt = arena_strndup(f->text + e->b->span.start, e->b->span.end - e->b->span.start);
                DiagFix *fx = diag_fix(d, "convert the index", "suggestion");
                diag_fix_edit(fx, e->b->span, arena_printf("@int_cast(usize, %s)", txt));
            }
        }
        if (e->b->is_const && t->kind == TY_ARRAY && !t->len_param && (e->b->cval < 0 || e->b->cval >= t->len))
            diag_error(e->b->span, "E0541", "index %s is out of bounds for %s", i128_str(e->b->cval), type_str(t));
        return e->type = elem ? elem : t_invalid;
    }
    // slice
    Expr **bounds[2] = {&e->b, &e->c};
    for (int k = 0; k < 2; k++) {
        if (!*bounds[k]) continue;
        Type *it = check_expr(*bounds[k], t_usize);
        if (it->kind == TY_UNTYPED_INT) coerce(bounds[k], t_usize);
        else if (it != t_usize && it->kind != TY_INVALID)
            diag_error((*bounds[k])->span, "E0540", "slice bound must have type usize, found %s", type_str(it));
    }
    if (!elem) return e->type = t_invalid;
    bool mut;
    if (e->op == IDX_ARRAY) {
        if (!is_place(e->a)) {
            diag_error(e->a->span, "E0704", "cannot slice a temporary array; bind it to a variable first");
            return e->type = t_invalid;
        }
        mark_addr_taken(e->a);
        mut = is_mut_place(e->a);
    } else {
        mut = t->mut;
    }
    return e->type = type_slice(elem, mut);
}

// ---------------------------------------------------------------------------
// Main dispatch
// ---------------------------------------------------------------------------
static Type *check_operand(Expr *e, Type *expected) {
    e->cat = CAT_VALUE;
    switch (e->kind) {
    case E_IDENT: resolve_ident(e); return e->type;
    case E_SELF:
        if (!C->tc.self) {
            diag_error(e->span, "E0205", "'Self' is only available inside a struct or union");
            e->type = t_invalid;
            return e->type;
        }
        e->cat = CAT_TYPE;
        e->tval = C->tc.self;
        return NULL;
    case E_INT:
        e->is_const = true;
        e->cval = (i128)e->ival;
        return e->type = t_untyped_int;
    case E_FLOAT:
        e->is_const = true;
        return e->type = t_untyped_float;
    case E_STRING: return e->type = type_slice(t_u8, false);
    case E_BOOL: return e->type = t_bool;
    case E_STRUCT_LIT: return check_struct_lit(e, expected);
    case E_ARRAY_LIT:
    case E_ARRAY_REPEAT: return check_array_lit(e, expected);
    case E_ERROR_VAL: {
        uint32_t code = error_code_of(e->name);
        if (!code) {
            Diag *d = diag_error(e->name_span, "E0650", "'error.%s' is not declared; add it to an 'error { ... }' block",
                                 e->name);
            (void)d;
        }
        e->cval = code;
        Decl *set = expected_set(expected);
        if (code && !set_has(set, e->name))
            diag_error(e->name_span, "E0652", "'error.%s' is not in error set %s", e->name, set->name);
        return e->type = type_errset(set);
    }
    case E_BUILTIN: return builtin_check(e, expected);
    case E_UNARY: return check_unary(e, expected);
    case E_BINARY: return check_binary(e, expected);
    case E_CATCH: return check_catch(e);
    case E_FIELD: check_field(e); return e->type;
    case E_DEREF: {
        Type *t = check_expr(e->a, NULL);
        if (t->kind == TY_PTR) {
            if (t->elem->kind == TY_VOID) {
                diag_error(e->span, "E0705", "cannot dereference *void; convert it with @ptr_cast first");
                return e->type = t_invalid;
            }
            return e->type = t->elem;
        }
        if (t->kind != TY_INVALID) diag_error(e->span, "E0705", "cannot dereference a value of type %s", type_str(t));
        return e->type = t_invalid;
    }
    case E_CALL: return check_call(e, expected);
    case E_INDEX:
    case E_SLICE: return check_index(e);
    case E_TRY: return check_try(e);
    case E_APPLY: check_apply(e); return e->type;
    case E_CONV: return e->type;
    }
    return e->type = t_invalid;
}

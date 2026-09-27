// sema_builtins.c - type checking of @builtin calls.
#include "sema_int.h"

static Type *targ(Expr *e, int i) {
    if (i >= e->bargs.len || !e->bargs.data[i].t) return t_invalid;
    return resolve_type(e->bargs.data[i].t, &C->tc);
}

static Expr **earg(Expr *e, int i) { return &e->bargs.data[i].e; }

static bool argc_ok(Expr *e) {
    const char *sig = builtin_info[e->index].sig;
    int want = 0;
    bool variadic = false;
    for (const char *p = sig; *p; p++) {
        if (*p == '*') variadic = true;
        else want++;
    }
    if (e->bargs.len < want || (!variadic && e->bargs.len > want)) {
        diag_error(e->span, "E0420", "@%s expects %s%d argument%s, found %d", e->name, variadic ? "at least " : "", want,
                   want == 1 ? "" : "s", e->bargs.len);
        for (int i = 0; i < e->bargs.len; i++)
            if (e->bargs.data[i].e) check_expr(e->bargs.data[i].e, NULL);
        return false;
    }
    return true;
}

static bool is_int_like(Type *t) {
    return t->kind == TY_INT || (t->kind == TY_PARAM && type_satisfies(t, C_INT)) || t->kind == TY_INVALID;
}
static bool is_float_like(Type *t) {
    return t->kind == TY_FLOAT || (t->kind == TY_PARAM && type_satisfies(t, C_FLOAT)) || t->kind == TY_INVALID;
}

// Check an operand; if it is an untyped literal, give it `hint` (or report).
static Type *operand(Expr **pe, Type *hint) {
    Type *t = check_expr(*pe, hint);
    if (ty_is_untyped(t)) {
        if (hint && (hint->kind == TY_INT || hint->kind == TY_FLOAT || hint->kind == TY_PARAM)) {
            coerce(pe, hint);
            return (*pe)->type;
        }
        finalize_untyped(pe);
        return t_invalid;
    }
    return t;
}

// Two operands of the same type.
static Type *pair(Expr *e, Type *expected) {
    Type *a = check_expr(*earg(e, 0), expected);
    Type *b = check_expr(*earg(e, 1), ty_is_untyped(a) ? expected : a);
    if (ty_is_untyped(a) && !ty_is_untyped(b)) {
        coerce(earg(e, 0), b);
        a = b;
    } else if (ty_is_untyped(b) && !ty_is_untyped(a)) {
        coerce(earg(e, 1), a);
        b = a;
    } else if (ty_is_untyped(a) && ty_is_untyped(b)) {
        if (expected && expected->kind != TY_ERRUNION) {
            coerce(earg(e, 0), expected);
            coerce(earg(e, 1), expected);
            return expected;
        }
        finalize_untyped(earg(e, 0));
        return t_invalid;
    }
    if (a != b && a->kind != TY_INVALID && b->kind != TY_INVALID) {
        Diag *d = diag_error(e->span, "E0412", "@%s operands have different types: %s and %s", e->name, type_str(a),
                             type_str(b));
        diag_types(d, type_str(a), type_str(b));
        return t_invalid;
    }
    return a;
}

static Type *mask_type(Type *v) {
    if (v->kind != TY_VEC) return t_invalid;
    return type_vec(type_int(v->elem->bits, true, false), v->lanes);
}

static bool lossless(Type *from, Type *to) {
    if (from == to) return true;
    if (from->kind == TY_INT && to->kind == TY_INT) {
        // usize/isize are at least 32 and at most 64 bits, portably.
        int fmax = from->is_size ? 64 : from->bits;
        int tmin = to->is_size ? 32 : to->bits;
        if (from->is_signed == to->is_signed) return fmax <= tmin;
        if (!from->is_signed && to->is_signed) return fmax < tmin;
        return false;
    }
    if (from->kind == TY_FLOAT && to->kind == TY_FLOAT) return from->bits <= to->bits;
    if (from->kind == TY_INT && to->kind == TY_FLOAT) {
        int mag = (from->is_size ? 64 : from->bits) - (from->is_signed ? 1 : 0);
        return mag <= (to->bits == 32 ? 24 : 53);
    }
    return false;
}

static int64_t const_lane(Expr *e, Expr **pe, int lanes) {
    Type *t = check_expr(*pe, t_u32);
    if (ty_is_untyped(t)) coerce(pe, t_u32);
    if (!(*pe)->is_const) {
        diag_error((*pe)->span, "E0820", "@%s lane index must be a constant", e->name);
        return 0;
    }
    if ((*pe)->cval < 0 || (*pe)->cval >= lanes) {
        diag_error((*pe)->span, "E0820", "lane index %lld out of range for %d lanes", (long long)(*pe)->cval, lanes);
        return 0;
    }
    return (int64_t)(*pe)->cval;
}

static bool require_vec(Expr *e, Expr *arg, Type *t) {
    if (t->kind == TY_VEC || t->kind == TY_INVALID) return true;
    diag_error(arg->span, "E0812", "@%s requires a SIMD vector, found %s", e->name, type_str(t));
    return false;
}

Type *builtin_check(Expr *e, Type *expected) {
    if (e->index < 0) {
        for (int i = 0; i < e->bargs.len; i++)
            if (e->bargs.data[i].e) check_expr(e->bargs.data[i].e, NULL);
        return e->type = t_invalid;
    }
    if (!argc_ok(e)) return e->type = t_invalid;
    Type *x = expected && expected->kind == TY_ERRUNION ? expected->elem : expected;
    switch ((BuiltinId)e->index) {
    case BI_SIZE_OF:
    case BI_ALIGN_OF: {
        Type *t = targ(e, 0);
        e->tval = t;
        if (!t->has_params && t->kind != TY_INVALID) {
            e->is_const = true;
            e->cval = e->index == BI_SIZE_OF ? type_size(t) : type_align(t);
        }
        return e->type = t_usize;
    }
    case BI_OFFSET_OF: {
        Type *t = targ(e, 0);
        e->tval = t;
        Expr *f = *earg(e, 1);
        if (f->kind != E_IDENT) {
            diag_error(f->span, "E0821", "@offset_of expects a field name");
            return e->type = t_usize;
        }
        if (t->kind != TY_STRUCT) {
            if (t->kind != TY_INVALID) diag_error(e->span, "E0821", "@offset_of requires a struct type");
            return e->type = t_usize;
        }
        int fi = find_field(t->decl, f->name);
        if (fi < 0) {
            diag_error(f->span, "E0404", "%s has no field '%s'", type_str(t), f->name);
            return e->type = t_usize;
        }
        e->index = BI_OFFSET_OF;
        f->index = fi;
        f->type = t_invalid;
        if (!t->has_params) {
            e->is_const = true;
            e->cval = type_field_offset(t, fi);
        }
        return e->type = t_usize;
    }
    case BI_MIN_VALUE:
    case BI_MAX_VALUE: {
        Type *t = targ(e, 0);
        e->tval = t;
        if (!is_int_like(t)) {
            diag_error(e->span, "E0822", "@%s requires an integer type", e->name);
            return e->type = t_invalid;
        }
        if (t->kind == TY_INT) {
            i128 lo, hi;
            int_range(t, &lo, &hi);
            e->is_const = true;
            e->cval = e->index == BI_MIN_VALUE ? lo : hi;
        }
        return e->type = t;
    }
    case BI_AS: {
        Type *t = targ(e, 0);
        e->tval = t;
        Type *s = check_expr(*earg(e, 1), t);
        if (ty_is_untyped(s)) {
            coerce(earg(e, 1), t);
            Expr *a = *earg(e, 1);
            e->is_const = a->is_const;
            e->cval = a->cval;
            e->fval = a->fval;
            return e->type = t;
        }
        if (s->kind != TY_INVALID && t->kind != TY_INVALID && !lossless(s, t)) {
            Diag *d = diag_error(e->span, "E0510", "@as cannot convert %s to %s without possible loss of information",
                                 type_str(s), type_str(t));
            if (s->kind == TY_INT && t->kind == TY_INT) {
                SourceFile *f = g_files.data[e->span.file];
                Expr *a = *earg(e, 1);
                const char *txt = arena_strndup(f->text + a->span.start, a->span.end - a->span.start);
                DiagFix *fx = diag_fix(d, "use a checked conversion", "suggestion");
                diag_fix_edit(fx, e->span, arena_printf("@int_cast(%s, %s)", type_str(t), txt));
            }
        }
        return e->type = t;
    }
    case BI_INT_CAST:
    case BI_TRUNCATE: {
        Type *t = targ(e, 0);
        e->tval = t;
        if (!is_int_like(t)) diag_error(e->span, "E0822", "@%s requires an integer target type", e->name);
        Type *s = check_expr(*earg(e, 1), NULL);
        if (ty_is_untyped(s)) {
            if (s->kind == TY_UNTYPED_INT && t->kind == TY_INT) {
                coerce(earg(e, 1), e->index == BI_TRUNCATE ? t_i64 : t);
            } else if (s->kind == TY_UNTYPED_INT && t->kind == TY_PARAM) {
                coerce(earg(e, 1), t_i64);
            } else {
                finalize_untyped(earg(e, 1));
            }
        } else if (!is_int_like(s)) {
            diag_error((*earg(e, 1))->span, "E0822", "@%s requires an integer operand, found %s", e->name, type_str(s));
        }
        return e->type = t;
    }
    case BI_INT_FROM_FLOAT: {
        Type *t = targ(e, 0);
        e->tval = t;
        if (!is_int_like(t)) diag_error(e->span, "E0822", "@int_from_float requires an integer target type");
        Type *s = operand(earg(e, 1), t_f64);
        if (!is_float_like(s)) diag_error((*earg(e, 1))->span, "E0822", "@int_from_float requires a float operand");
        return e->type = t;
    }
    case BI_FLOAT_FROM_INT: {
        Type *t = targ(e, 0);
        e->tval = t;
        if (!is_float_like(t)) diag_error(e->span, "E0822", "@float_from_int requires a float target type");
        Type *s = operand(earg(e, 1), t_i64);
        if (!is_int_like(s)) diag_error((*earg(e, 1))->span, "E0822", "@float_from_int requires an integer operand");
        return e->type = t;
    }
    case BI_FLOAT_CAST: {
        Type *t = targ(e, 0);
        e->tval = t;
        if (!is_float_like(t)) diag_error(e->span, "E0822", "@float_cast requires a float target type");
        Type *s = operand(earg(e, 1), t_f64);
        if (!is_float_like(s)) diag_error((*earg(e, 1))->span, "E0822", "@float_cast requires a float operand");
        return e->type = t;
    }
    case BI_BIT_CAST: {
        Type *t = targ(e, 0);
        e->tval = t;
        Type *s = operand(earg(e, 1), t);
        if (t->kind == TY_INVALID || s->kind == TY_INVALID) return e->type = t;
        bool ok_kinds = !ty_is_aggregate(t) && !ty_is_aggregate(s) && t->kind != TY_VOID && s->kind != TY_BOOL &&
                        t->kind != TY_BOOL;
        if (!ok_kinds) {
            diag_error(e->span, "E0512", "@bit_cast supports numeric, pointer and vector types only");
        } else if (t->has_params || s->has_params) {
            if (t != s) diag_error(e->span, "E0512", "@bit_cast between generic types of unknown size");
        } else if (type_size(t) != type_size(s)) {
            diag_error(e->span, "E0512", "@bit_cast size mismatch: %s is %lld bytes, %s is %lld bytes", type_str(s),
                       (long long)type_size(s), type_str(t), (long long)type_size(t));
        }
        return e->type = t;
    }
    case BI_INT_FROM_PTR: {
        Type *s = check_expr(*earg(e, 0), NULL);
        if (s->kind != TY_PTR && s->kind != TY_INVALID)
            diag_error(e->span, "E0822", "@int_from_ptr requires a pointer, found %s", type_str(s));
        return e->type = t_usize;
    }
    case BI_PTR_FROM_INT: {
        Type *t = targ(e, 0);
        e->tval = t;
        if (t->kind != TY_PTR && t->kind != TY_INVALID) diag_error(e->span, "E0822", "@ptr_from_int requires a pointer type");
        check_expr_to(earg(e, 1), t_usize);
        return e->type = t;
    }
    case BI_PTR_CAST: {
        Type *t = targ(e, 0);
        e->tval = t;
        if (t->kind != TY_PTR && t->kind != TY_INVALID) diag_error(e->span, "E0822", "@ptr_cast requires a pointer type");
        Type *s = check_expr(*earg(e, 1), NULL);
        if (s->kind != TY_PTR && s->kind != TY_INVALID)
            diag_error((*earg(e, 1))->span, "E0822", "@ptr_cast requires a pointer operand, found %s", type_str(s));
        if (s->kind == TY_PTR && t->kind == TY_PTR && t->mut && !s->mut)
            diag_error(e->span, "E0706", "@ptr_cast cannot add 'mut' to a pointer");
        return e->type = t;
    }
    case BI_SLICE_FROM_PTR: {
        Type *p = check_expr(*earg(e, 0), NULL);
        check_expr_to(earg(e, 1), t_usize);
        if (p->kind != TY_PTR) {
            if (p->kind != TY_INVALID) diag_error(e->span, "E0822", "@slice_from_ptr requires a pointer");
            return e->type = t_invalid;
        }
        if (p->elem->kind == TY_VOID) {
            diag_error(e->span, "E0822", "@slice_from_ptr cannot create a slice of void");
            return e->type = t_invalid;
        }
        return e->type = type_slice(p->elem, p->mut);
    }
    case BI_CAST_SLICE: {
        Type *t = targ(e, 0);
        e->tval = t;
        Type *s = check_expr(*earg(e, 1), NULL);
        if (s->kind != TY_SLICE) {
            if (s->kind != TY_INVALID) diag_error(e->span, "E0822", "@cast_slice requires a slice, found %s", type_str(s));
            return e->type = t_invalid;
        }
        if (t->kind == TY_VOID) {
            diag_error(e->span, "E0822", "@cast_slice cannot produce a slice of void");
            return e->type = t_invalid;
        }
        return e->type = type_slice(t, s->mut);
    }
    case BI_MEM_COPY:
    case BI_MEM_SET: {
        Type *d = check_expr(*earg(e, 0), NULL);
        if (d->kind != TY_SLICE || !d->mut) {
            if (d->kind != TY_INVALID)
                diag_error((*earg(e, 0))->span, "E0822", "@%s destination must be a []mut slice, found %s", e->name,
                           type_str(d));
            check_expr(*earg(e, 1), NULL);
            return e->type = t_void;
        }
        if (e->index == BI_MEM_COPY) check_expr_to(earg(e, 1), type_slice(d->elem, false));
        else check_expr_to(earg(e, 1), d->elem);
        return e->type = t_void;
    }
    case BI_MEM_EQ: {
        Type *a = check_expr(*earg(e, 0), NULL);
        if (a->kind != TY_SLICE) {
            if (a->kind != TY_INVALID) diag_error(e->span, "E0822", "@mem_eq requires slices");
            check_expr(*earg(e, 1), NULL);
            return e->type = t_bool;
        }
        if (a->mut) coerce(earg(e, 0), type_slice(a->elem, false));
        check_expr_to(earg(e, 1), type_slice(a->elem, false));
        return e->type = t_bool;
    }
    case BI_MEMORY_SIZE: return e->type = t_usize;
    case BI_MEMORY_GROW:
        check_expr_to(earg(e, 0), t_usize);
        return e->type = type_errunion(t_usize);
    case BI_CLZ:
    case BI_CTZ:
    case BI_POPCOUNT: {
        Type *t = operand(earg(e, 0), x);
        if (!is_int_like(t)) diag_error(e->span, "E0822", "@%s requires an integer, found %s", e->name, type_str(t));
        return e->type = t;
    }
    case BI_ROTL:
    case BI_ROTR: {
        Type *t = operand(earg(e, 0), x);
        if (!is_int_like(t)) diag_error(e->span, "E0822", "@%s requires an integer, found %s", e->name, type_str(t));
        Type *k = check_expr(*earg(e, 1), t_u32);
        if (ty_is_untyped(k)) coerce(earg(e, 1), t_u32);
        else if (!(k->kind == TY_INT && !k->is_signed) && k->kind != TY_INVALID)
            diag_error((*earg(e, 1))->span, "E0525", "rotation count must be unsigned");
        return e->type = t;
    }
    case BI_ADD_SAT:
    case BI_SUB_SAT:
    case BI_MUL_SAT: {
        Type *t = pair(e, x);
        bool ok = is_int_like(t) ||
                  (e->index != BI_MUL_SAT && ty_is_vec_int(t) && (t->elem->bits == 8 || t->elem->bits == 16));
        if (!ok) diag_error(e->span, "E0822", "@%s requires integers, found %s", e->name, type_str(t));
        return e->type = t;
    }
    case BI_ADD_CHECKED:
    case BI_SUB_CHECKED:
    case BI_MUL_CHECKED: {
        Type *t = pair(e, x);
        if (!is_int_like(t)) diag_error(e->span, "E0822", "@%s requires integers, found %s", e->name, type_str(t));
        return e->type = type_errunion(t);
    }
    case BI_MIN:
    case BI_MAX: {
        Type *t = pair(e, x);
        bool ok = ty_is_num(t) || ty_is_vec_float(t) || ty_is_vec_int(t) || t->kind == TY_INVALID ||
                  (t->kind == TY_PARAM && type_satisfies(t, C_ORD));
        if (ty_is_vec_int(t) && t->elem->bits == 64) ok = false;
        if (!ok) diag_error(e->span, "E0822", "@%s cannot be applied to %s", e->name, type_str(t));
        return e->type = t;
    }
    case BI_ABS: {
        Type *t = operand(earg(e, 0), x);
        bool ok = (t->kind == TY_INT && t->is_signed) || t->kind == TY_FLOAT || ty_is_vec_float(t) ||
                  (ty_is_vec_int(t) && t->elem->is_signed) || t->kind == TY_INVALID ||
                  (t->kind == TY_PARAM && (type_satisfies(t, C_SIGNED) || type_satisfies(t, C_FLOAT)));
        if (!ok) diag_error(e->span, "E0822", "@abs requires a signed integer or float, found %s", type_str(t));
        return e->type = t;
    }
    case BI_SQRT:
    case BI_FLOOR:
    case BI_CEIL:
    case BI_TRUNC:
    case BI_NEAREST: {
        Type *t = operand(earg(e, 0), x && (x->kind == TY_FLOAT || x->kind == TY_PARAM) ? x : t_f64);
        if (!is_float_like(t) && !ty_is_vec_float(t))
            diag_error(e->span, "E0822", "@%s requires a float, found %s", e->name, type_str(t));
        return e->type = t;
    }
    case BI_FMA: {
        Type *t = pair(e, x);
        check_expr_to(earg(e, 2), t);
        if (!is_float_like(t) && !ty_is_vec_float(t))
            diag_error(e->span, "E0822", "@fma requires floats, found %s", type_str(t));
        return e->type = t;
    }
    case BI_REM:
    case BI_COPYSIGN: {
        Type *t = pair(e, x);
        if (!is_float_like(t)) diag_error(e->span, "E0822", "@%s requires floats, found %s", e->name, type_str(t));
        return e->type = t;
    }
    case BI_SPLAT: {
        if (!x || (x->kind != TY_VEC && x->kind != TY_INVALID)) {
            diag_error(e->span, "E0820", "@splat needs a vector type from context, e.g. let v: f32x4 = @splat(x);");
            check_expr(*earg(e, 0), NULL);
            return e->type = t_invalid;
        }
        if (x->kind == TY_INVALID) {
            check_expr(*earg(e, 0), NULL);
            return e->type = t_invalid;
        }
        check_expr_to(earg(e, 0), x->elem);
        return e->type = x;
    }
    case BI_VEC: {
        Type *v = targ(e, 0);
        e->tval = v;
        if (v->kind != TY_VEC) {
            if (v->kind != TY_INVALID) diag_error(e->span, "E0812", "@vec requires a lane-typed vector type");
            for (int i = 1; i < e->bargs.len; i++) check_expr(e->bargs.data[i].e, NULL);
            return e->type = t_invalid;
        }
        if (e->bargs.len - 1 != v->lanes)
            diag_error(e->span, "E0420", "@vec(%s, ...) needs exactly %d lane values, found %d", type_str(v), v->lanes,
                       e->bargs.len - 1);
        for (int i = 1; i < e->bargs.len; i++) check_expr_to(&e->bargs.data[i].e, v->elem);
        return e->type = v;
    }
    case BI_EXTRACT_LANE: {
        Type *v = check_expr(*earg(e, 0), NULL);
        if (!require_vec(e, *earg(e, 0), v) || v->kind == TY_INVALID) {
            check_expr(*earg(e, 1), NULL);
            return e->type = t_invalid;
        }
        const_lane(e, earg(e, 1), v->lanes);
        return e->type = v->elem;
    }
    case BI_REPLACE_LANE: {
        Type *v = check_expr(*earg(e, 0), x);
        if (!require_vec(e, *earg(e, 0), v) || v->kind == TY_INVALID) {
            check_expr(*earg(e, 1), NULL);
            check_expr(*earg(e, 2), NULL);
            return e->type = t_invalid;
        }
        const_lane(e, earg(e, 1), v->lanes);
        check_expr_to(earg(e, 2), v->elem);
        return e->type = v;
    }
    case BI_LOAD: {
        Type *v = targ(e, 0);
        e->tval = v;
        if (!ty_is_simd(v) && v->kind != TY_INVALID) diag_error(e->span, "E0812", "@load requires a vector type");
        Type *s = check_expr(*earg(e, 1), NULL);
        if (s->kind != TY_SLICE && s->kind != TY_INVALID)
            diag_error((*earg(e, 1))->span, "E0822", "@load reads from a slice, found %s", type_str(s));
        return e->type = v;
    }
    case BI_STORE: {
        Type *s = check_expr(*earg(e, 0), NULL);
        if ((s->kind != TY_SLICE || !s->mut) && s->kind != TY_INVALID)
            diag_error((*earg(e, 0))->span, "E0822", "@store writes to a []mut slice, found %s", type_str(s));
        Type *v = check_expr(*earg(e, 1), NULL);
        if (!ty_is_simd(v) && v->kind != TY_INVALID) diag_error((*earg(e, 1))->span, "E0812", "@store requires a vector value");
        return e->type = t_void;
    }
    case BI_SHUFFLE: {
        Type *t = pair(e, x);
        if (!require_vec(e, *earg(e, 0), t) || t->kind == TY_INVALID) {
            check_expr(*earg(e, 2), NULL);
            return e->type = t_invalid;
        }
        Expr *idx = *earg(e, 2);
        if (idx->kind != E_ARRAY_LIT || idx->args.len != t->lanes) {
            diag_error(idx->span, "E0820", "@shuffle needs an array literal of %d constant lane indices", t->lanes);
            check_expr(idx, NULL);
            return e->type = t;
        }
        for (int i = 0; i < idx->args.len; i++) {
            Expr **pi = &idx->args.data[i];
            check_expr(*pi, t_u32);
            if (!(*pi)->is_const || (*pi)->cval < 0 || (*pi)->cval >= 2 * t->lanes)
                diag_error((*pi)->span, "E0820", "shuffle index must be a constant in 0..%d", 2 * t->lanes - 1);
            if (ty_is_untyped((*pi)->type)) coerce(pi, t_u32);
        }
        idx->type = type_array(t_u32, t->lanes);
        return e->type = t;
    }
    case BI_SWIZZLE: {
        Type *t = pair(e, x);
        if (t->kind != TY_INVALID && !(t->kind == TY_VEC && t->elem->bits == 8))
            diag_error(e->span, "E0812", "@swizzle requires u8x16 or i8x16 operands");
        return e->type = t;
    }
    case BI_LANES_EQ:
    case BI_LANES_NE:
    case BI_LANES_LT:
    case BI_LANES_LE:
    case BI_LANES_GT:
    case BI_LANES_GE: {
        Type *t = pair(e, NULL);
        if (!require_vec(e, *earg(e, 0), t)) return e->type = t_invalid;
        return e->type = mask_type(t);
    }
    case BI_SELECT: {
        Type *m = check_expr(*earg(e, 0), NULL);
        Type *a = check_expr(*earg(e, 1), x);
        check_expr_to(earg(e, 2), a);
        if (!ty_is_simd(a) && a->kind != TY_INVALID) diag_error(e->span, "E0812", "@select requires vector operands");
        if (m->kind != TY_INVALID && !(ty_is_vec_int(m) || m->kind == TY_V128))
            diag_error((*earg(e, 0))->span, "E0812", "@select mask must be an integer vector");
        return e->type = a;
    }
    case BI_ANY_TRUE:
    case BI_ALL_TRUE: {
        Type *t = check_expr(*earg(e, 0), NULL);
        if (!ty_is_simd(t) && t->kind != TY_INVALID) diag_error(e->span, "E0812", "@%s requires a vector", e->name);
        if (e->index == BI_ALL_TRUE && t->kind == TY_V128) diag_error(e->span, "E0812", "@all_true requires a lane-typed vector");
        return e->type = t_bool;
    }
    case BI_BITMASK: {
        Type *t = check_expr(*earg(e, 0), NULL);
        if (!ty_is_vec_int(t) && t->kind != TY_INVALID) diag_error(e->span, "E0812", "@bitmask requires an integer vector");
        return e->type = t_u32;
    }
    case BI_REDUCE_ADD: {
        Type *t = check_expr(*earg(e, 0), NULL);
        if (!require_vec(e, *earg(e, 0), t) || t->kind == TY_INVALID) return e->type = t_invalid;
        return e->type = t->elem;
    }
    case BI_DOT: {
        Type *t = pair(e, NULL);
        if (t->kind == TY_INVALID) return e->type = t_invalid;
        if (ty_is_vec_float(t)) return e->type = t->elem;
        if (t->kind == TY_VEC && t->elem == t_i16) return e->type = type_vec(t_i32, 4);
        diag_error(e->span, "E0812", "@dot requires float vectors or i16x8, found %s", type_str(t));
        return e->type = t_invalid;
    }
    case BI_TRAP: return e->type = t_never;
    case BI_FENCE: return e->type = t_void;
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
    case BI_ATOMIC_NOTIFY: {
        // First argument: pointer to an integer (spec §12.8). Everything but
        // load, wait and notify needs *mut.
        BuiltinId id = (BuiltinId)e->index;
        bool need_mut = id != BI_ATOMIC_LOAD && id != BI_ATOMIC_WAIT && id != BI_ATOMIC_NOTIFY;
        Type *p = check_expr(*earg(e, 0), NULL);
        Type *t = t_invalid;
        if (p->kind == TY_PTR && (p->elem->kind == TY_INT || (p->elem->kind == TY_PARAM && type_satisfies(p->elem, C_INT)))) {
            t = p->elem;
            if (need_mut && !p->mut)
                diag_error((*earg(e, 0))->span, "E0706", "@%s needs a *mut pointer, found %s", e->name, type_str(p));
        } else if (p->kind != TY_INVALID) {
            diag_error((*earg(e, 0))->span, "E0823", "@%s needs a pointer to an integer, found %s", e->name, type_str(p));
        }
        if ((id == BI_ATOMIC_WAIT || id == BI_ATOMIC_NOTIFY) && t->kind == TY_INT && t->bits != 32) {
            diag_error((*earg(e, 0))->span, "E0823", "@%s works on 32-bit integers (*i32 or *u32), found %s", e->name,
                       type_str(p));
        }
        if ((id == BI_ATOMIC_WAIT || id == BI_ATOMIC_NOTIFY) && t->kind == TY_PARAM) {
            diag_error((*earg(e, 0))->span, "E0823", "@%s works on 32-bit integers (*i32 or *u32)", e->name);
        }
        switch (id) {
        case BI_ATOMIC_LOAD: return e->type = t;
        case BI_ATOMIC_STORE:
            check_expr_to(earg(e, 1), t);
            return e->type = t_void;
        case BI_ATOMIC_CAS:
            check_expr_to(earg(e, 1), t);
            check_expr_to(earg(e, 2), t);
            return e->type = t;
        case BI_ATOMIC_WAIT:
            check_expr_to(earg(e, 1), t);
            check_expr_to(earg(e, 2), t_i64);
            return e->type = t_u32;
        case BI_ATOMIC_NOTIFY:
            check_expr_to(earg(e, 1), t_u32);
            return e->type = t_u32;
        default:
            check_expr_to(earg(e, 1), t);
            return e->type = t;  // read-modify-write: the old value
        }
    }
    case BI_ERROR_NAME:
        check_expr_to(earg(e, 0), t_error);
        return e->type = type_slice(t_u8, false);
    default: break;
    }
    return e->type = t_invalid;
}

// sema_stmt.c - statements and function bodies.
#include <errno.h>
#include "sema_int.h"

void check_return_value_ext(Expr **pe, Span sp);

// ---------------------------------------------------------------------------
// Scopes and locals
// ---------------------------------------------------------------------------
void scope_push(void) { vec_push(C->marks, C->scope.len); }
void scope_pop(void) { C->scope.len = vec_pop(C->marks); }

Local *scope_lookup(Sym name) {
    for (int i = C->scope.len - 1; i >= 0; i--)
        if (C->scope.data[i].name == name) return C->scope.data[i].local;
    return NULL;
}

Local *declare_local(Sym name, Span sp, Type *t, bool mut, bool is_param) {
    Local *prev = scope_lookup(name);
    Span prev_span = prev ? prev->span : sp;
    bool clash = prev != NULL;
    const char *what = "a variable";
    if (!clash && lookup_gparam(&C->tc, name)) {
        clash = true;
        what = "a generic parameter";
    }
    if (!clash) {
        Decl *d = module_lookup(C->tc.mod, name);
        if (d) {
            clash = true;
            prev_span = d->name_span;
            what = "a top-level declaration";
        }
    }
    if (!clash) {
        Decl *d = symmap_get(&C->tc.mod->imports, name);
        if (d) {
            clash = true;
            prev_span = d->name_span;
            what = "an import";
        }
    }
    if (clash) {
        Diag *dg = diag_error(sp, "E0204", "'%s' shadows %s; choose a different name", name, what);
        diag_note(dg, prev_span, "'%s' is declared here", name);
    }
    Local *l = NEW(Local);
    l->name = name;
    l->span = sp;
    l->type = t;
    l->mut = mut;
    l->is_param = is_param;
    if (C->fn) {
        l->index = C->fn->locals.len;
        vec_push(C->fn->locals, l);
    }
    ScopeEntry se = {name, l};
    vec_push(C->scope, se);
    return l;
}

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------
bool check_block(Stmt *b) {
    scope_push();
    bool div = false, warned = false;
    for (int i = 0; i < b->stmts.len; i++) {
        Stmt *s = b->stmts.data[i];
        if (div && !warned) {
            diag_warning(s->span, "W0601", "unreachable statement");
            warned = true;
        }
        if (check_stmt(s)) div = true;
    }
    scope_pop();
    b->diverges = div;
    return div;
}

static bool is_leaving_catch(Expr *e) {
    return e && e->kind == E_CATCH && (e->hkind == H_RETURN || e->hkind == H_BREAK || e->hkind == H_CONTINUE);
}

static void check_assign(Stmt *s) {
    Expr *lhs = s->e;
    Type *lt = check_expr(lhs, NULL);
    if (lt->kind == TY_INVALID) {
        check_expr(s->e2, NULL);
        return;
    }
    if (!is_place(lhs)) {
        diag_error(lhs->span, "E0706", "the left-hand side of an assignment must be a variable, field, element or dereference");
        check_expr(s->e2, NULL);
        return;
    }
    if (!is_mut_place(lhs)) {
        Diag *d = diag_error(lhs->span, "E0701", "cannot assign: this place is not mutable");
        Expr *r = lhs;
        while ((r->kind == E_FIELD && r->self_mode != SELF_DEREF) || (r->kind == E_INDEX && r->op == 0)) r = r->a;
        if (r->kind == E_IDENT && r->local) {
            if (r->local->is_param) {
                diag_note(d, r->local->span, "parameters are immutable; copy '%s' into a 'let mut' first", r->local->name);
            } else {
                diag_note(d, r->local->span, "'%s' is declared without 'mut'", r->local->name);
                DiagFix *fx = diag_fix(d, "declare the variable as mutable", "machine");
                diag_fix_edit(fx, (Span){r->local->span.file, r->local->span.start, r->local->span.start}, "mut ");
            }
        }
    }
    if (s->op == TK_EQ) {
        check_expr_to(&s->e2, lt);
        return;
    }
    // compound assignment: lhs op= rhs, with the semantics of lhs op rhs
    int op;
    switch (s->op) {
    case TK_PLUS_EQ: op = TK_PLUS; break;
    case TK_MINUS_EQ: op = TK_MINUS; break;
    case TK_STAR_EQ: op = TK_STAR; break;
    case TK_SLASH_EQ: op = TK_SLASH; break;
    case TK_PERCENT_EQ: op = TK_PERCENT; break;
    case TK_PLUS_PCT_EQ: op = TK_PLUS_PCT; break;
    case TK_MINUS_PCT_EQ: op = TK_MINUS_PCT; break;
    case TK_STAR_PCT_EQ: op = TK_STAR_PCT; break;
    case TK_AMP_EQ: op = TK_AMP; break;
    case TK_PIPE_EQ: op = TK_PIPE; break;
    case TK_CARET_EQ: op = TK_CARET; break;
    case TK_SHL_EQ: op = TK_SHL; break;
    default: op = TK_SHR; break;
    }
    s->e3 = NEW(Expr);  // synthetic binary expr describing the operation (for codegen)
    s->e3->kind = E_BINARY;
    s->e3->op = op;
    s->e3->span = s->span;
    s->e3->a = lhs;
    s->e3->b = s->e2;
    s->e3->type = lt;
    s->e3->cat = CAT_VALUE;
    if (op == TK_SHL || op == TK_SHR) {
        Type *r = check_expr(s->e2, NULL);
        if (r->kind == TY_UNTYPED_INT) coerce(&s->e2, t_u32);
        else if (ty_is_vec_int(r) && !r->elem->is_signed) {
            // per lane (spec §13.1)
            if (lt->kind != TY_INVALID && !(ty_is_vec_int(lt) && lt->lanes == r->lanes && lt->elem->bits == r->elem->bits))
                diag_error(s->e2->span, "E0412",
                           "per-lane shift counts must have the shape of the shifted vector: %s and %s", type_str(lt),
                           type_str(r));
        } else if (!(r->kind == TY_INT && !r->is_signed) && r->kind != TY_INVALID &&
                   !(r->kind == TY_PARAM && type_satisfies(r, C_UNSIGNED)))
            diag_error(s->e2->span, "E0525", "shift count must be an unsigned integer, found %s", type_str(r));
        if (lt->kind != TY_INT && !ty_is_vec_int(lt) && !(lt->kind == TY_PARAM && type_satisfies(lt, C_INT)))
            diag_error(s->span, "E0513", "shift requires an integer operand, found %s", type_str(lt));
        s->e3->b = s->e2;
        return;
    }
    check_expr_to(&s->e2, lt);
    s->e3->b = s->e2;
    // reuse the binary-operator validation by type-checking a probe
    Expr probe = *s->e3;
    probe.a = NEW(Expr);
    probe.a->kind = E_CONV;
    probe.a->type = lt;
    probe.a->cat = CAT_VALUE;
    probe.a->span = lhs->span;
    probe.b = NEW(Expr);
    probe.b->kind = E_CONV;
    probe.b->type = lt;
    probe.b->cat = CAT_VALUE;
    probe.b->span = s->e2->span;
    probe.type = NULL;
    check_expr(&probe, NULL);
}

static void check_match(Stmt *s, bool *diverges) {
    Type *t = check_expr(s->e, NULL);
    if (ty_is_untyped(t)) t = finalize_untyped(&s->e);
    bool all_div = true;
    bool has_else = false;
    int narms = s->arms.len;
    if (t->kind == TY_UNION || t->kind == TY_ENUM) {
        Decl *u = t->decl;
        bool is_enum = t->kind == TY_ENUM;
        bool *seen = NEWN(bool, u->variants.len + 1);
        for (int i = 0; i < narms; i++) {
            MatchArm *a = &s->arms.data[i];
            if (has_else) diag_warning(a->span, "W0631", "unreachable match arm after 'else'");
            scope_push();
            if (a->kind == P_ELSE) {
                has_else = true;
            } else if (a->kind == P_VARIANT) {
                int v = find_variant(u, a->name);
                if (v < 0) {
                    diag_error(a->span, "E0632", "%s has no %s '%s'", type_str(t), is_enum ? "member" : "variant", a->name);
                } else {
                    a->variant = v;
                    a->ival = u->variants.data[v].value;
                    if (seen[v])
                        diag_error(a->span, "E0631", "%s '%s' is matched more than once", is_enum ? "member" : "variant",
                                   a->name);
                    seen[v] = true;
                    Type *pt = is_enum ? NULL : type_variant(t, v);
                    if (a->bind) {
                        if (!pt)
                            diag_error(a->bind_span, "E0633", "%s '%s' has no payload to bind",
                                       is_enum ? "enum member" : "variant", a->name);
                        else a->bind_local = declare_local(a->bind, a->bind_span, pt, false, false);
                    }
                }
            } else {
                diag_error(a->span, "E0634", "this pattern cannot match a value of %s type %s", is_enum ? "enum" : "union",
                           type_str(t));
            }
            if (!check_block(a->body)) all_div = false;
            scope_pop();
        }
        if (!has_else) {
            CharVec missing = {0};
            int nm = 0;
            for (int v = 0; v < u->variants.len; v++) {
                if (seen[v]) continue;
                const char *n = u->variants.data[v].name;
                if (nm++) {
                    vec_push(missing, ',');
                    vec_push(missing, ' ');
                }
                vec_push(missing, '.');
                while (*n) vec_push(missing, *n++);
            }
            vec_push(missing, 0);
            if (nm) {
                Diag *d = diag_error(s->span, "E0630", "non-exhaustive match: missing %s", missing.data);
                (void)d;
            }
            free(missing.data);
        }
    } else if (t->kind == TY_INT || t->kind == TY_BOOL || t->kind == TY_ERROR) {
        Vec(i128) seenv = {0};
        bool seen_true = false, seen_false = false;
        for (int i = 0; i < narms; i++) {
            MatchArm *a = &s->arms.data[i];
            if (has_else) diag_warning(a->span, "W0631", "unreachable match arm after 'else'");
            scope_push();
            i128 key = 0;
            bool keyed = false;
            if (a->kind == P_ELSE) {
                has_else = true;
            } else if (a->kind == P_INT && t->kind == TY_INT) {
                if (!fits_type(a->ival, t)) diag_error(a->span, "E0503", "pattern does not fit in %s", type_str(t));
                key = a->ival;
                keyed = true;
            } else if (a->kind == P_BOOL && t->kind == TY_BOOL) {
                if (a->bval ? seen_true : seen_false) diag_error(a->span, "E0631", "pattern is matched more than once");
                if (a->bval) seen_true = true;
                else seen_false = true;
            } else if (a->kind == P_ERROR && t->kind == TY_ERROR) {
                uint32_t code = error_code_of(a->name);
                if (!code) diag_error(a->span, "E0650", "'error.%s' is not declared", a->name);
                a->error_code = code;
                key = code;
                keyed = true;
            } else {
                diag_error(a->span, "E0634", "this pattern cannot match a value of type %s", type_str(t));
            }
            if (keyed) {
                for (int k = 0; k < seenv.len; k++)
                    if (seenv.data[k] == key) diag_error(a->span, "E0631", "pattern is matched more than once");
                vec_push(seenv, key);
            }
            if (!check_block(a->body)) all_div = false;
            scope_pop();
        }
        free(seenv.data);
        bool exhaustive = has_else || (t->kind == TY_BOOL && seen_true && seen_false);
        if (!exhaustive) diag_error(s->span, "E0630", "non-exhaustive match: add an 'else' arm");
    } else {
        if (t->kind != TY_INVALID)
            diag_error(s->e->span, "E0634",
                       "cannot match on a value of type %s (only unions, enums, integers, bool and error)",
                       type_str(t));
        for (int i = 0; i < narms; i++) {
            scope_push();
            check_block(s->arms.data[i].body);
            scope_pop();
        }
        all_div = false;
        has_else = true;
    }
    *diverges = narms > 0 && all_div;
}

bool check_stmt(Stmt *s) {
    bool div = false;
    switch (s->kind) {
    case S_LET: {
        Type *t;
        if (s->undef) {
            // `= undefined`: the type must be written, and the variable writable
            t = t_invalid;
            if (s->te) {
                t = resolve_type(s->te, &C->tc);
                if (t->kind == TY_VOID) {
                    diag_error(s->te->span, "E0409", "a variable cannot have type void");
                    t = t_invalid;
                }
            } else {
                Diag *d = diag_error(s->e->span, "E0414", "'undefined' gives '%s' no type; add a type annotation", s->name);
                DiagFix *fx = diag_fix(d, "annotate the type", "placeholder");
                diag_fix_edit(fx, (Span){s->name_span.file, s->name_span.end, s->name_span.end}, ": <type>");
            }
            if (!s->mut) {
                Diag *d = diag_error(s->e->span, "E0413",
                                     "'%s' is left undefined but immutable, so it could never be written; use 'let mut'",
                                     s->name);
                DiagFix *fx = diag_fix(d, "make it mutable", "machine");
                diag_fix_edit(fx, (Span){s->name_span.file, s->name_span.start, s->name_span.start}, "mut ");
            }
            s->e->type = t;
            s->local = declare_local(s->name, s->name_span, t, s->mut, false);
            break;
        }
        if (s->te) {
            t = resolve_type(s->te, &C->tc);
            if (t->kind == TY_VOID) {
                diag_error(s->te->span, "E0409", "a variable cannot have type void");
                t = t_invalid;
            }
            check_expr_to(&s->e, t);
        } else {
            t = check_expr(s->e, NULL);
            if (ty_is_untyped(t)) {
                Diag *d = diag_error(s->e->span, "E0502",
                                     "cannot infer the type of '%s' from a literal; add a type annotation", s->name);
                DiagFix *fx = diag_fix(d, "annotate the type", "placeholder");
                diag_fix_edit(fx, (Span){s->name_span.file, s->name_span.end, s->name_span.end}, ": <type>");
                t = t_invalid;
            } else if (t->kind == TY_VOID) {
                diag_error(s->e->span, "E0409", "cannot bind a value of type void");
                t = t_invalid;
            } else if (t->kind == TY_NEVER) {
                t = t_invalid;
            }
        }
        if (s->e->type && s->e->type->kind == TY_NEVER) div = true;
        s->local = declare_local(s->name, s->name_span, t, s->mut, false);
        break;
    }
    case S_EXPR: {
        Type *t = check_expr(s->e, NULL);
        if (t->kind == TY_NEVER) div = true;
        else if (ty_is_untyped(t) || (t->kind != TY_VOID && t->kind != TY_INVALID)) {
            if (t->kind == TY_ERRUNION) {
                Diag *d = diag_error(s->e->span, "E0620", "error union result of type %s must be handled", type_str(t));
                DiagFix *fx = diag_fix(d, "propagate the error", "suggestion");
                diag_fix_edit(fx, (Span){s->e->span.file, s->e->span.end, s->e->span.end}, "!");
            } else {
                Diag *d = diag_error(s->e->span, "E0620", "result of type %s is discarded; write '_ = ...' to discard it",
                                     type_str(t));
                DiagFix *fx = diag_fix(d, "discard explicitly", "machine");
                diag_fix_edit(fx, (Span){s->e->span.file, s->e->span.start, s->e->span.start}, "_ = ");
            }
        }
        break;
    }
    case S_ASSIGN: check_assign(s); break;
    case S_DISCARD: {
        Type *t = check_expr(s->e, NULL);
        if (ty_is_untyped(t)) finalize_untyped(&s->e);
        else if (t->kind == TY_VOID) diag_error(s->e->span, "E0622", "nothing to discard: the expression has type void");
        else if (t->kind == TY_ERRUNION)
            diag_error(s->e->span, "E0621", "an error union cannot be discarded; handle it with '!' or 'catch'");
        break;
    }
    case S_IF: {
        check_expr_to(&s->e, t_bool);
        bool a = check_block(s->body);
        bool b = false;
        if (s->els) b = s->els->kind == S_IF ? check_stmt(s->els) : check_block(s->els);
        div = s->els && a && b;
        break;
    }
    case S_WHILE:
        check_expr_to(&s->e, t_bool);
        C->loop_depth++;
        vec_push(C->loop_breaks, false);
        check_block(s->body);
        C->loop_breaks.len--;
        C->loop_depth--;
        break;
    case S_LOOP: {
        C->loop_depth++;
        vec_push(C->loop_breaks, false);
        check_block(s->body);
        bool broke = vec_pop(C->loop_breaks);
        C->loop_depth--;
        div = !broke;
        break;
    }
    case S_FOR: {
        Type *vt;
        if (s->e2) {
            Type *a = check_expr(s->e, NULL);
            Type *b = check_expr(s->e2, ty_is_untyped(a) ? NULL : a);
            if (ty_is_untyped(a) && !ty_is_untyped(b)) {
                coerce(&s->e, b);
                a = b;
            } else if (!ty_is_untyped(a) && ty_is_untyped(b)) {
                coerce(&s->e2, a);
                b = a;
            } else if (ty_is_untyped(a) && ty_is_untyped(b)) {
                Diag *d = diag_error(s->e->span, "E0502", "cannot infer the type of the range; give a bound a type, e.g. @as(usize, 0)");
                (void)d;
                a = b = t_invalid;
            }
            if (a != b && a->kind != TY_INVALID && b->kind != TY_INVALID) {
                Diag *d = diag_error(s->e2->span, "E0412", "range bounds have different types: %s and %s", type_str(a),
                                     type_str(b));
                diag_types(d, type_str(a), type_str(b));
            }
            if (a->kind != TY_INT && a->kind != TY_INVALID && !(a->kind == TY_PARAM && type_satisfies(a, C_INT)))
                diag_error(s->e->span, "E0412", "range bounds must be integers, found %s", type_str(a));
            vt = a;
        } else {
            Type *t = check_expr(s->e, NULL);
            if (t->kind == TY_ARRAY) {
                vt = t->elem;
                if (!is_place(s->e)) {
                    // iterating a temporary array is fine: codegen spills it
                }
            } else if (t->kind == TY_SLICE) {
                vt = t->elem;
            } else {
                if (t->kind != TY_INVALID)
                    diag_error(s->e->span, "E0412", "'for x in' needs an array or slice, found %s (use lo..hi for ranges)",
                               type_str(t));
                vt = t_invalid;
            }
        }
        scope_push();
        s->local = declare_local(s->name, s->name_span, vt, false, false);
        C->loop_depth++;
        vec_push(C->loop_breaks, false);
        check_block(s->body);
        C->loop_breaks.len--;
        C->loop_depth--;
        scope_pop();
        break;
    }
    case S_MATCH: check_match(s, &div); break;
    case S_RETURN:
        check_return_value_ext(s->e ? &s->e : NULL, s->span);
        div = true;
        break;
    case S_BREAK:
    case S_CONTINUE:
        if (C->loop_depth == 0) {
            diag_error(s->span, "E0604", "'%s' outside of a loop", s->kind == S_BREAK ? "break" : "continue");
        } else if (C->defer_depth > 0 && C->loop_depth <= C->defer_loop_base) {
            diag_error(s->span, "E0640", "control flow cannot leave a deferred statement");
        }
        if (s->kind == S_BREAK && C->loop_breaks.len) vec_last(C->loop_breaks) = true;
        div = true;
        break;
    case S_DEFER: {
        int saved_base = C->defer_loop_base;
        C->defer_depth++;
        C->defer_loop_base = C->loop_depth;
        if (s->body->kind == S_BLOCK) check_block(s->body);
        else {
            check_stmt(s->body);
            if (s->body->kind == S_EXPR && is_leaving_catch(s->body->e))
                diag_error(s->body->span, "E0640", "control flow cannot leave a deferred statement");
        }
        C->defer_depth--;
        C->defer_loop_base = saved_base;
        break;
    }
    case S_BLOCK: div = check_block(s); break;
    }
    s->diverges = div;
    return div;
}

void check_fn_body(Decl *fn) {
    if (fn->checked) return;
    fn->checked = true;
    resolve_decl(fn);
    check_fn_attr(fn);
    FnCtx ctx = {0};
    ctx.tc = (TypeCtx){fn->module, fn->owner, fn, fn->owner ? fn->owner->type : NULL};
    ctx.fn = fn;
    ctx.ret = fn->ret_type;
    FnCtx *saved = C;
    C = &ctx;
    scope_push();
    for (int i = 0; i < fn->params.len; i++) {
        Param *p = &fn->params.data[i];
        p->local = declare_local(p->name, p->span, fn->param_types[i], false, true);
    }
    bool div = check_block(fn->body);
    Type *r = fn->ret_type;
    if (!div && r->kind != TY_VOID && r->kind != TY_INVALID && !(r->kind == TY_ERRUNION && r->elem->kind == TY_VOID)) {
        Span end = {fn->body->span.file, fn->body->span.end > 0 ? fn->body->span.end - 1 : 0, fn->body->span.end};
        Diag *d = diag_error(end, "E0601", "missing return: function '%s' must return %s on every path", fn->name,
                             type_str(r));
        diag_note(d, fn->name_span, "'%s' declared here", fn->name);
    }
    scope_pop();
    free(ctx.scope.data);
    free(ctx.marks.data);
    free(ctx.loop_breaks.data);
    C = saved;
    gate_fn(fn);
}

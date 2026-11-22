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
        Decl *d = module_lookup_import(C->tc.mod, sp.file, name);
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
    if (C) vec_push(C->locals, l);
    ScopeEntry se = {name, l};
    vec_push(C->scope, se);
    return l;
}

MoveSnap move_snap_take(void) {
    MoveSnap s = {0};
    if (!C || C->locals.len == 0) return s;
    s.len = C->locals.len;
    s.moved = calloc((size_t)s.len, sizeof(bool));
    s.spans = calloc((size_t)s.len, sizeof(Span));
    s.origins = calloc((size_t)s.len, sizeof(Local *));
    s.origin_fields = calloc((size_t)s.len, sizeof(uint64_t));
    for (int i = 0; i < s.len; i++) {
        s.moved[i] = C->locals.data[i]->moved;
        s.spans[i] = C->locals.data[i]->moved_span;
        s.origins[i] = C->locals.data[i]->origin_local;
        s.origin_fields[i] = C->locals.data[i]->origin_fields;
    }
    return s;
}

void move_snap_restore(const MoveSnap *s) {
    if (!C) return;
    for (int i = 0; i < s->len && i < C->locals.len; i++) {
        C->locals.data[i]->moved = s->moved[i];
        C->locals.data[i]->moved_span = s->spans[i];
        C->locals.data[i]->origin_local = s->origins ? s->origins[i] : NULL;
        C->locals.data[i]->origin_fields = s->origin_fields ? s->origin_fields[i] : 0ULL;
    }
}

void move_snap_join(const MoveSnap *s) {
    if (!C) return;
    for (int i = 0; i < s->len && i < C->locals.len; i++) {
        Local *l = C->locals.data[i];
        if (!l->moved && s->moved[i]) {
            l->moved = true;
            l->moved_span = s->spans[i];
        }
        if (!l->origin_local && s->origins && s->origins[i]) {
            l->origin_local = s->origins[i];
        }
        if (s->origin_fields) {
            l->origin_fields |= s->origin_fields[i];
        }
    }
}

void move_snap_free(MoveSnap *s) {
    free(s->moved);
    free(s->spans);
    free(s->origins);
    free(s->origin_fields);
    s->moved = NULL;
    s->spans = NULL;
    s->origins = NULL;
    s->origin_fields = NULL;
    s->len = 0;
}

static void move_snap_or_into(MoveSnap *acc, int prefix_len) {
    if (!C) return;
    for (int i = 0; i < prefix_len && i < C->locals.len; i++) {
        Local *l = C->locals.data[i];
        if (l->moved && !acc->moved[i]) {
            acc->moved[i] = true;
            acc->spans[i] = l->moved_span;
        }
        if (l->origin_local && acc->origins && !acc->origins[i]) {
            acc->origins[i] = l->origin_local;
        }
        if (acc->origin_fields) {
            acc->origin_fields[i] |= l->origin_fields;
        }
    }
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
    return e && (e->kind == E_CATCH || e->kind == E_ORELSE) &&
           (e->hkind == H_RETURN || e->hkind == H_BREAK || e->hkind == H_CONTINUE);
}

static void check_assign(Stmt *s) {
    Expr *lhs = s->e;
    bool plain_ident = (s->op == TK_EQ && lhs->kind == E_IDENT);
    if (plain_ident && C) C->suppress_moved_ident = true;
    Type *lt = check_expr(lhs, NULL);
    if (C) C->suppress_moved_ident = false;
    if (lt->kind == TY_INVALID) {
        check_expr(s->e2, NULL);
        if (plain_ident && lhs->local) {
            lhs->local->moved = false;
            lhs->local->moved_span = (Span){0};
        }
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
        if (s->e2->type == lt) check_consume_nocopy(s->e2, false);
        if (plain_ident && lhs->local) {
            lhs->local->moved = false;
            lhs->local->moved_span = (Span){0};
        }
        if (lhs->kind == E_IDENT && lhs->local) {
            lhs->local->origin_local = expr_local_origin_mask(s->e2, &lhs->local->origin_fields);
        } else if (lhs->kind == E_FIELD && lhs->op == FK_STRUCT && lhs->self_mode != SELF_DEREF &&
                   lhs->a && lhs->a->kind == E_IDENT && lhs->a->local) {
            Local *l = lhs->a->local;
            Local *src = expr_local_origin(s->e2);
            if (src != NULL) {
                if (lhs->index >= 0 && lhs->index < 64) l->origin_fields |= (1ULL << lhs->index);
                else l->origin_fields = ~0ULL;
                if (!l->origin_local) l->origin_local = src;
            } else if (lhs->index >= 0 && lhs->index < 64) {
                l->origin_fields &= ~(1ULL << lhs->index);
                if (l->origin_fields == 0) l->origin_local = NULL;
            }
        } else {
            Local *src = expr_local_origin(s->e2);
            Local *dst_local = place_local(lhs);
            if (dst_local != NULL) {
                if (src != NULL) {
                    dst_local->origin_local = src;
                    dst_local->origin_fields = ~0ULL;
                }
            } else {
                Local *po = place_origin(lhs);
                if (po != NULL) {
                    if (src != NULL) {
                        po->origin_local = src;
                        po->origin_fields = ~0ULL;
                    }
                } else if (src != NULL) {
                    diag_error(s->e2->span, "E0710",
                               "this points into local '%s', which cannot be stored in a global or caller-reachable place",
                               src->name);
                }
            }
        }
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
        } else if (r->kind == TY_PARAM && type_satisfies(r, C_INT_VEC) && r->constraint_lane &&
                   ((r->constraint_lane->kind == TY_INT && !r->constraint_lane->is_signed) ||
                    (r->constraint_lane->kind == TY_PARAM && type_satisfies(r->constraint_lane, C_UNSIGNED)))) {
            if (lt->kind != TY_INVALID && r != lt)
                diag_error(s->e2->span, "E0412",
                           "per-lane shift counts must have the shape of the shifted vector: %s and %s", type_str(lt),
                           type_str(r));
        } else if (!(r->kind == TY_INT && !r->is_signed) && r->kind != TY_INVALID &&
                   !(r->kind == TY_PARAM && type_satisfies(r, C_UNSIGNED)))
            diag_error(s->e2->span, "E0525", "shift count must be an unsigned integer, found %s", type_str(r));
        if (lt->kind != TY_INT && !ty_is_vec_int(lt) && lt->kind != TY_INVALID &&
            !(lt->kind == TY_PARAM && (type_satisfies(lt, C_INT) || type_satisfies(lt, C_INT_VEC)))) {
            if (lt->kind == TY_PARAM)
                diag_error(s->span, "E0461", "operator '%s' is not permitted on '%s' (constraint %s)",
                           tok_kind_name((TokKind)op), lt->name, param_constraint_str(lt));
            else
                diag_error(s->span, "E0513", "shift requires an integer operand, found %s", type_str(lt));
        }
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


static void enter_loop(Stmt *s, const MoveSnap *entry) {
    C->loop_depth++;
    LoopInfo li = {0};
    li.label = s->label;
    li.local_base = entry->len;
    if (li.local_base > 0) {
        li.entry_moved = calloc((size_t)li.local_base, sizeof(bool));
        li.entry_spans = calloc((size_t)li.local_base, sizeof(Span));
        li.break_moved = calloc((size_t)li.local_base, sizeof(bool));
        li.break_spans = calloc((size_t)li.local_base, sizeof(Span));
        li.break_origins = calloc((size_t)li.local_base, sizeof(Local *));
        li.break_origin_fields = calloc((size_t)li.local_base, sizeof(uint64_t));
        memcpy(li.entry_moved, entry->moved, (size_t)li.local_base * sizeof(bool));
        memcpy(li.entry_spans, entry->spans, (size_t)li.local_base * sizeof(Span));
    }
    vec_push(C->loops, li);
}

static LoopInfo leave_loop(void) {
    C->loop_depth--;
    return vec_pop(C->loops);
}

static void free_loop_info(LoopInfo *li) {
    free(li->entry_moved);
    free(li->entry_spans);
    free(li->break_moved);
    free(li->break_spans);
    free(li->break_origins);
    free(li->break_origin_fields);
}

static void check_loop_iter_end(const LoopInfo *L) {
    if (C->flow_dead) return;
    for (int i = 0; i < L->local_base && i < C->locals.len; i++) {
        Local *loc = C->locals.data[i];
        if (!L->entry_moved[i] && loc->moved) {
            Diag *dg = diag_error(loc->moved_span, "E0709", "use of moved value '%s'", loc->name);
            diag_note(dg, loc->moved_span, "value moved here");
        }
    }
}

int check_jump_target(Sym label, Span sp, bool is_break) {
    if (C->loop_depth == 0) {
        diag_error(sp, "E0604", "'%s' outside of a loop", is_break ? "break" : "continue");
        return -1;
    }
    int d = 0;
    if (label) {
        int n = C->loops.len, i = n;
        while (i > 0 && !(C->loops.data[i - 1].label && !strcmp(C->loops.data[i - 1].label, label))) i--;
        if (i == 0) {
            diag_error(sp, "E0605", "no enclosing loop is labeled '%s'", label);
            return -1;
        }
        d = n - i;
    }
    if (C->defer_depth > 0 && C->loop_depth - d <= C->defer_loop_base) {
        diag_error(sp, "E0640", "control flow cannot leave a deferred statement");
        return -1;
    }
    LoopInfo *L = &C->loops.data[C->loops.len - 1 - d];
    if (is_break) {
        L->has_break = true;
        if (!C->flow_dead) {
            L->break_snap_init = true;
            for (int i = 0; i < L->local_base && i < C->locals.len; i++) {
                Local *loc = C->locals.data[i];
                if (loc->moved && !L->break_moved[i]) {
                    L->break_moved[i] = true;
                    L->break_spans[i] = loc->moved_span;
                }
                if (loc->origin_local && !L->break_origins[i]) {
                    L->break_origins[i] = loc->origin_local;
                }
                L->break_origin_fields[i] |= loc->origin_fields;
            }
        }
    } else if (!C->flow_dead) {
        for (int i = 0; i < L->local_base && i < C->locals.len; i++) {
            Local *loc = C->locals.data[i];
            if (!L->entry_moved[i] && loc->moved) {
                Diag *dg = diag_error(sp, "E0709", "use of moved value '%s'", loc->name);
                diag_note(dg, loc->moved_span, "value moved here");
            }
        }
    }
    return d;
}

static void check_match(Stmt *s, bool *diverges) {
    Type *t = check_expr(s->e, NULL);
    if (ty_is_untyped(t)) t = finalize_untyped(&s->e);
    if (t->kind != TY_PTR && type_is_nocopy(t) && is_place(s->e) &&
        !(s->e->kind == E_IDENT && s->e->local && s->e->local->moved)) {
        bool has_bind = false;
        for (int i = 0; i < s->arms.len; i++) {
            if (s->arms.data[i].bind) {
                has_bind = true;
                break;
            }
        }
        if (has_bind) diag_error(s->e->span, "E0708", "cannot copy nocopy value of type %s", type_str(t));
    }
    bool all_div = true;
    bool has_else = false;
    int narms = s->arms.len;
    bool dead_before = C->flow_dead;
    MoveSnap before = move_snap_take();
    MoveSnap acc = {0};
    if (before.len > 0) {
        acc.len = before.len;
        acc.moved = calloc((size_t)acc.len, sizeof(bool));
        acc.spans = calloc((size_t)acc.len, sizeof(Span));
        acc.origins = calloc((size_t)acc.len, sizeof(Local *));
        acc.origin_fields = calloc((size_t)acc.len, sizeof(uint64_t));
    }
    bool any_live = false;
    Type *ref_ptr = (t->kind == TY_PTR && (t->elem->kind == TY_UNION || t->elem->kind == TY_OPT)) ? t : NULL;
    Type *base = ref_ptr ? t->elem : t;
    if (base->kind == TY_OPT) {
        bool seen[2] = {false, false};
        for (int i = 0; i < narms; i++) {
            MatchArm *a = &s->arms.data[i];
            if (!a->alt) {
                move_snap_restore(&before);
                C->flow_dead = dead_before;
            }
            if (has_else && !a->alt) diag_warning(a->span, "W0631", "unreachable match arm after 'else'");
            scope_push();
            if (a->kind == P_ELSE) {
                has_else = true;
            } else if (a->kind == P_VARIANT) {
                if (!strcmp(a->name, "none")) {
                    a->variant = 0;
                    if (seen[0]) diag_error(a->span, "E0631", "variant 'none' is matched more than once");
                    seen[0] = true;
                    if (a->bind) diag_error(a->bind_span, "E0633", "variant 'none' has no payload to bind");
                } else if (!strcmp(a->name, "some")) {
                    a->variant = 1;
                    if (seen[1]) diag_error(a->span, "E0631", "variant 'some' is matched more than once");
                    seen[1] = true;
                    if (a->bind) {
                        if (a->alt || (i + 1 < narms && s->arms.data[i + 1].alt))
                            diag_error(a->bind_span, "E0635", "a match arm with several patterns cannot bind a payload");
                        else {
                            Type *pt = ref_ptr ? type_ptr(base->elem, ref_ptr->mut) : base->elem;
                            a->bind_local = declare_local(a->bind, a->bind_span, pt, false, false);
                            a->bind_local->weakened = s->e->weakened;
                            if (ref_ptr) {
                                a->bind_local->origin_local = expr_local_origin(s->e);
                                a->bind_local->origin_fields = a->bind_local->origin_local ? ~0ULL : 0ULL;
                            } else {
                                a->bind_local->origin_local = expr_local_origin_mask(s->e, &a->bind_local->origin_fields);
                            }
                        }
                    }
                } else {
                    diag_error(a->span, "E0632", "%s has no variant '%s' (expected 'none' or 'some')",
                               type_str(base), a->name);
                }
            } else {
                diag_error(a->span, "E0634", "this pattern cannot match a value of optional type %s", type_str(base));
            }
            if (!a->alt) {
                bool arm_div = check_block(a->body);
                if (!arm_div) all_div = false;
                if (!dead_before && !arm_div && !C->flow_dead) {
                    any_live = true;
                    move_snap_or_into(&acc, before.len);
                }
            }
            scope_pop();
        }
        if (!has_else) {
            if (!seen[0] && !seen[1]) diag_error(s->span, "E0630", "non-exhaustive match: missing .none, .some");
            else if (!seen[0]) diag_error(s->span, "E0630", "non-exhaustive match: missing .none");
            else if (!seen[1]) diag_error(s->span, "E0630", "non-exhaustive match: missing .some");
        }
    } else if (base->kind == TY_UNION || base->kind == TY_ENUM) {
        Decl *u = base->decl;
        bool is_enum = base->kind == TY_ENUM;
        bool *seen = NEWN(bool, u->variants.len + 1);
        for (int i = 0; i < narms; i++) {
            MatchArm *a = &s->arms.data[i];
            if (!a->alt) {
                move_snap_restore(&before);
                C->flow_dead = dead_before;
            }
            if (has_else && !a->alt) diag_warning(a->span, "W0631", "unreachable match arm after 'else'");
            scope_push();
            if (a->kind == P_ELSE) {
                has_else = true;
            } else if (a->kind == P_VARIANT) {
                int v = find_variant(u, a->name);
                if (v < 0) {
                    diag_error(a->span, "E0632", "%s has no %s '%s'", type_str(base), is_enum ? "member" : "variant", a->name);
                } else {
                    a->variant = v;
                    a->ival = u->variants.data[v].value;
                    if (seen[v])
                        diag_error(a->span, "E0631", "%s '%s' is matched more than once", is_enum ? "member" : "variant",
                                   a->name);
                    seen[v] = true;
                    Type *pt = is_enum ? NULL : type_variant(base, v);
                    if (a->bind) {
                        if (!pt)
                            diag_error(a->bind_span, "E0633", "%s '%s' has no payload to bind",
                                       is_enum ? "enum member" : "variant", a->name);
                        else if (a->alt || (i + 1 < narms && s->arms.data[i + 1].alt))
                            diag_error(a->bind_span, "E0635", "a match arm with several patterns cannot bind a payload");
                        else {
                            Type *bt = ref_ptr ? type_ptr(pt, ref_ptr->mut) : pt;
                            a->bind_local = declare_local(a->bind, a->bind_span, bt, false, false);
                            a->bind_local->weakened = s->e->weakened;
                            a->bind_local->origin_local = expr_local_origin(s->e);
                            a->bind_local->origin_fields = a->bind_local->origin_local ? ~0ULL : 0ULL;
                        }
                    }
                }
            } else {
                diag_error(a->span, "E0634", "this pattern cannot match a value of %s type %s", is_enum ? "enum" : "union",
                           type_str(base));
            }
            if (!a->alt) {
                bool arm_div = check_block(a->body);
                if (!arm_div) all_div = false;
                if (!dead_before && !arm_div && !C->flow_dead) {
                    any_live = true;
                    move_snap_or_into(&acc, before.len);
                }
            }
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
    } else if (t->kind == TY_INT || t->kind == TY_BOOL || t->kind == TY_ERROR ||
               (t->kind == TY_PARAM && t->is_error_param)) {
        Vec(Sym) seen_names = {0};
        Vec(i128) seenv = {0};
        bool seen_true = false, seen_false = false;
        for (int i = 0; i < narms; i++) {
            MatchArm *a = &s->arms.data[i];
            if (!a->alt) {
                move_snap_restore(&before);
                C->flow_dead = dead_before;
            }
            if (has_else && !a->alt) diag_warning(a->span, "W0631", "unreachable match arm after 'else'");
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
            } else if (a->kind == P_ERROR && (t->kind == TY_ERROR || (t->kind == TY_PARAM && t->is_error_param))) {
                uint32_t code = error_code_of(a->name);
                if (!code) diag_error(a->span, "E0650", "'error.%s' is not declared", a->name);
                else if (t->kind == TY_ERROR && !set_has(t->decl, a->name))
                    diag_error(a->span, "E0652", "'error.%s' is not in error set %s", a->name, t->decl->name);
                vec_push(seen_names, a->name);
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
            if (!a->alt) {
                bool arm_div = check_block(a->body);
                if (!arm_div) all_div = false;
                if (!dead_before && !arm_div && !C->flow_dead) {
                    any_live = true;
                    move_snap_or_into(&acc, before.len);
                }
            }
            scope_pop();
        }
        free(seenv.data);
        bool exhaustive = has_else || (t->kind == TY_BOOL && seen_true && seen_false);
        if (!exhaustive && t->kind == TY_ERROR && t->decl) {
            // a named error set: every error it holds, or else
            CharVec missing = {0};
            int nm = 0;
            for (int v = 0; v < t->decl->set_names.len; v++) {
                Sym n = t->decl->set_names.data[v];
                bool hit = false;
                for (int k = 0; k < seen_names.len; k++)
                    if (seen_names.data[k] == n) hit = true;
                if (hit) continue;
                if (nm++) {
                    vec_push(missing, ',');
                    vec_push(missing, ' ');
                }
                for (const char *c = "error."; *c; c++) vec_push(missing, *c);
                while (*n) vec_push(missing, *n++);
            }
            vec_push(missing, 0);
            if (nm) diag_error(s->span, "E0630", "non-exhaustive match: missing %s", missing.data);
            free(missing.data);
        } else if (!exhaustive) {
            diag_error(s->span, "E0630", "non-exhaustive match: add an 'else' arm");
        }
        free(seen_names.data);
    } else {
        if (t->kind != TY_INVALID)
            diag_error(s->e->span, "E0634",
                       "cannot match on a value of type %s (only unions, optionals, *union, *optional, enums, integers, bool and error)",
                       type_str(t));
        for (int i = 0; i < narms; i++) {
            if (s->arms.data[i].alt) continue;
            move_snap_restore(&before);
            C->flow_dead = dead_before;
            scope_push();
            bool arm_div = check_block(s->arms.data[i].body);
            if (!dead_before && !arm_div && !C->flow_dead) {
                any_live = true;
                move_snap_or_into(&acc, before.len);
            }
            scope_pop();
        }
        all_div = false;
        has_else = true;
    }
    if (narms == 0) {
        move_snap_restore(&before);
        C->flow_dead = dead_before;
    } else if (dead_before || !any_live) {
        move_snap_restore(&before);
        C->flow_dead = true;
    } else {
        move_snap_restore(&acc);
        C->flow_dead = false;
    }
    move_snap_free(&acc);
    move_snap_free(&before);
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
        else if (s->e->type == t) check_consume_nocopy(s->e, false);
        s->local = declare_local(s->name, s->name_span, t, s->mut, false);
        if (s->e) {
            s->local->weakened = s->e->weakened;
            s->local->origin_local = expr_local_origin_mask(s->e, &s->local->origin_fields);
        }
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
        else check_consume_nocopy(s->e, false);
        if (t->kind == TY_NEVER && C) C->flow_dead = true;
        break;
    }
    case S_IF: {
        check_expr_to(&s->e, t_bool);
        bool dead_before = C->flow_dead;
        MoveSnap before = move_snap_take();
        C->flow_dead = dead_before;
        bool a = check_block(s->body);
        bool then_dead = dead_before || a || C->flow_dead;
        MoveSnap after_then = move_snap_take();
        move_snap_restore(&before);
        C->flow_dead = dead_before;
        bool b = false;
        bool else_dead = dead_before;
        if (s->els) {
            b = s->els->kind == S_BLOCK ? check_block(s->els) : check_stmt(s->els);
            else_dead = dead_before || b || C->flow_dead;
        }
        if (dead_before) {
            move_snap_restore(&before);
            C->flow_dead = true;
        } else if (then_dead && else_dead) {
            C->flow_dead = true;
        } else if (then_dead) {
            C->flow_dead = false;
        } else if (else_dead) {
            move_snap_restore(&after_then);
            C->flow_dead = false;
        } else {
            move_snap_join(&after_then);
            C->flow_dead = false;
        }
        move_snap_free(&after_then);
        move_snap_free(&before);
        div = s->els && a && b;
        break;
    }
    case S_WHILE: {
        bool dead_before = C->flow_dead;
        MoveSnap pre_cond = move_snap_take();
        check_expr_to(&s->e, t_bool);
        MoveSnap post_cond = move_snap_take();
        enter_loop(s, &pre_cond);
        bool bdiv = check_block(s->body);
        LoopInfo li = leave_loop();
        if (!dead_before && !bdiv) check_loop_iter_end(&li);
        if (dead_before) {
            move_snap_restore(&pre_cond);
            C->flow_dead = true;
        } else {
            bool falls = !bdiv && !C->flow_dead;
            for (int i = 0; i < li.local_base && i < C->locals.len; i++) {
                Local *loc = C->locals.data[i];
                bool m = post_cond.moved[i];
                Span sp = post_cond.spans[i];
                Local *orig = post_cond.origins ? post_cond.origins[i] : NULL;
                uint64_t fmask = post_cond.origin_fields ? post_cond.origin_fields[i] : 0ULL;
                if (li.break_snap_init && li.break_moved[i] && !m) {
                    m = true;
                    sp = li.break_spans[i];
                }
                if (li.break_snap_init) {
                    if (li.break_origins[i] && !orig) orig = li.break_origins[i];
                    fmask |= li.break_origin_fields[i];
                }
                if (falls && loc->moved && !m) {
                    m = true;
                    sp = loc->moved_span;
                }
                if (falls) {
                    if (loc->origin_local && !orig) orig = loc->origin_local;
                    fmask |= loc->origin_fields;
                }
                loc->moved = m;
                loc->moved_span = sp;
                loc->origin_local = orig;
                loc->origin_fields = fmask;
            }
            C->flow_dead = false;
        }
        free_loop_info(&li);
        move_snap_free(&post_cond);
        move_snap_free(&pre_cond);
        break;
    }
    case S_LOOP: {
        bool dead_before = C->flow_dead;
        MoveSnap entry = move_snap_take();
        enter_loop(s, &entry);
        bool bdiv = check_block(s->body);
        LoopInfo li = leave_loop();
        if (!dead_before && !bdiv) check_loop_iter_end(&li);
        if (dead_before) {
            move_snap_restore(&entry);
            C->flow_dead = true;
        } else if (li.break_snap_init) {
            for (int i = 0; i < li.local_base && i < C->locals.len; i++) {
                C->locals.data[i]->moved = li.break_moved[i];
                C->locals.data[i]->moved_span = li.break_spans[i];
                C->locals.data[i]->origin_local = li.break_origins[i];
                C->locals.data[i]->origin_fields = li.break_origin_fields[i];
            }
            C->flow_dead = false;
        } else {
            C->flow_dead = true;
        }
        div = !li.has_break;
        free_loop_info(&li);
        move_snap_free(&entry);
        break;
    }
    case S_FOR: {
        Type *vt;
        Local *for_origin = NULL;
        if (s->e2) {
            if (s->ref_iter) {
                diag_error(s->span, "E0706", "by-reference 'for &' cannot iterate over a range");
                check_expr(s->e, NULL);
                check_expr(s->e2, NULL);
                vt = t_invalid;
            } else {
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
            }
        } else {
            Type *t = check_expr(s->e, NULL);
            if (s->ref_iter) {
                if (t->kind == TY_SLICE) {
                    if (s->ref_mut && !t->mut)
                        diag_error(s->e->span, "E0701", "cannot take '*mut' elements from immutable slice %s",
                                   type_str(t));
                    vt = type_ptr(t->elem, s->ref_mut);
                    for_origin = expr_local_origin(s->e);
                } else if (t->kind == TY_ARRAY) {
                    if (s->ref_mut) {
                        if (!is_place(s->e))
                            diag_error(s->e->span, "E0703", "'&mut' cannot take the address of a temporary");
                        else if (!is_mut_place(s->e))
                            diag_error(s->e->span, "E0701", "cannot take '*mut' elements from an immutable array");
                    }
                    if (is_place(s->e)) {
                        mark_addr_taken(s->e);
                        for_origin = place_origin(s->e);
                    }
                    vt = type_ptr(t->elem, s->ref_mut);
                } else {
                    if (t->kind != TY_INVALID)
                        diag_error(s->e->span, "E0412", "'for &%s in' needs an array or slice, found %s",
                                   s->ref_mut ? "mut x" : "x", type_str(t));
                    vt = t_invalid;
                }
            } else {
                if (t->kind == TY_ARRAY) {
                    vt = t->elem;
                    for_origin = expr_local_origin(s->e);
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
                if (type_is_nocopy(vt)) {
                    Diag *dg = diag_error(s->name_span, "E0708", "cannot copy nocopy element of type %s in value loop",
                                          type_str(vt));
                    bool can_mut = (t->kind == TY_SLICE && t->mut && !s->e->weakened) ||
                                   (t->kind == TY_ARRAY && is_place(s->e) && is_mut_place(s->e) && !s->e->weakened);
                    if (can_mut) {
                        DiagFix *fx = diag_fix(dg, "iterate by mutable pointer", "machine");
                        diag_fix_edit(fx, s->name_span, arena_printf("&mut %s", s->name));
                    }
                    DiagFix *fx = diag_fix(dg, "iterate by pointer", "machine");
                    diag_fix_edit(fx, s->name_span, arena_printf("&%s", s->name));
                }
            }
        }
        bool dead_before = C->flow_dead;
        MoveSnap entry = move_snap_take();
        scope_push();
        s->local = declare_local(s->name, s->name_span, vt, false, false);
        s->local->origin_local = for_origin;
        s->local->origin_fields = for_origin ? ~0ULL : 0ULL;
        enter_loop(s, &entry);
        bool bdiv = check_block(s->body);
        LoopInfo li = leave_loop();
        if (!dead_before && !bdiv) check_loop_iter_end(&li);
        if (dead_before) {
            move_snap_restore(&entry);
            C->flow_dead = true;
        } else {
            bool falls = !bdiv && !C->flow_dead;
            for (int i = 0; i < li.local_base && i < C->locals.len; i++) {
                Local *loc = C->locals.data[i];
                bool m = entry.moved[i];
                Span sp = entry.spans[i];
                Local *orig = entry.origins ? entry.origins[i] : NULL;
                uint64_t fmask = entry.origin_fields ? entry.origin_fields[i] : 0ULL;
                if (li.break_snap_init && li.break_moved[i] && !m) {
                    m = true;
                    sp = li.break_spans[i];
                }
                if (li.break_snap_init) {
                    if (li.break_origins[i] && !orig) orig = li.break_origins[i];
                    fmask |= li.break_origin_fields[i];
                }
                if (falls && loc->moved && !m) {
                    m = true;
                    sp = loc->moved_span;
                }
                if (falls) {
                    if (loc->origin_local && !orig) orig = loc->origin_local;
                    fmask |= loc->origin_fields;
                }
                loc->moved = m;
                loc->moved_span = sp;
                loc->origin_local = orig;
                loc->origin_fields = fmask;
            }
            C->flow_dead = false;
        }
        free_loop_info(&li);
        move_snap_free(&entry);
        scope_pop();
        break;
    }
    case S_MATCH: check_match(s, &div); break;
    case S_RETURN:
        check_return_value_ext(s->e ? &s->e : NULL, s->span);
        div = true;
        break;
    case S_BREAK:
    case S_CONTINUE: {
        int d = check_jump_target(s->label, s->label ? s->label_span : s->span, s->kind == S_BREAK);
        s->jump_depth = d < 0 ? 0 : d;
        div = true;
        break;
    }
    case S_DEFER: {
        if (s->on_error && (!C->ret || C->ret->kind != TY_ERRUNION))
            diag_error(s->span, "E0641", "errdefer needs a function that returns an error union (!T)");
        bool dead_before = C->flow_dead;
        MoveSnap before = move_snap_take();
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
        move_snap_restore(&before);
        move_snap_free(&before);
        C->flow_dead = dead_before;
        break;
    }
    case S_BLOCK: div = check_block(s); break;
    }
    if (div) C->flow_dead = true;
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
    if (fn->is_infer_ret) fn->inferring_ret = true;
    FnCtx *saved = C;
    C = &ctx;
    scope_push();
    for (int i = 0; i < fn->params.len; i++) {
        Param *p = &fn->params.data[i];
        p->local = declare_local(p->name, p->span, fn->param_types[i], false, true);
    }
    bool div = check_block(fn->body);
    if (fn->is_infer_ret) {
        Type *r0 = fn->ret_type;
        Type *elem = r0->kind == TY_ERRUNION ? r0->elem : r0;
        if (ctx.ret_err_open) {
            fn->ret_type = type_errunion(elem);
        } else {
            Decl *set = intern_anon_errset(ctx.ret_errs.data, ctx.ret_errs.len);
            fn->ret_type = type_errunion_set(elem, set);
        }
        fn->type = type_fn(fn->param_types, fn->params.len, fn->ret_type);
        fn->inferring_ret = false;
    } else if (!fn->gparams.len && (!fn->owner || !fn->owner->gparams.len) &&
               fn->ret_type->kind == TY_ERRUNION && fn->ret_type->decl && fn->ret_type->decl->is_anon &&
               !ctx.ret_err_open && fn->ret_te && fn->ret_te->kind == TE_ERRUNION && fn->ret_te->set &&
               fn->ret_te->set->kind == TE_ERRSET_INLINE && fn->ret_te->elem) {
        Decl *decl = fn->ret_type->decl;
        Vec(Sym) kept = {0};
        for (int i = 0; i < decl->set_names.len; i++) {
            Sym s = decl->set_names.data[i];
            bool found = false;
            for (int j = 0; j < ctx.ret_errs.len; j++) {
                if (ctx.ret_errs.data[j] == s) {
                    found = true;
                    break;
                }
            }
            if (found) vec_push(kept, s);
        }
        if (kept.len < decl->set_names.len) {
            Span sp = fn->ret_te->set->span;
            Diag *dg = diag_warning(sp, "W0655", "declared error set contains errors this function can never return");
            if (kept.len > 0) {
                CharVec b = {0};
                vec_push(b, '{');
                vec_push(b, ' ');
                for (int i = 0; i < kept.len; i++) {
                    if (i) {
                        vec_push(b, ',');
                        vec_push(b, ' ');
                    }
                    for (const char *p = kept.data[i]; *p; p++) vec_push(b, *p);
                }
                vec_push(b, ' ');
                vec_push(b, '}');
                vec_push(b, 0);
                DiagFix *fx = diag_fix(dg, "narrow the error set", "machine");
                diag_fix_edit(fx, sp, arena_strndup(b.data, strlen(b.data)));
                free(b.data);
            } else {
                DiagFix *fx = diag_fix(dg, "remove the unused error set", "suggestion");
                diag_fix_edit(fx, (Span){sp.file, sp.start, fn->ret_te->elem->span.start}, "");
            }
        }
        free(kept.data);
    }
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
    free(ctx.loops.data);
    free(ctx.locals.data);
    free(ctx.ret_errs.data);
    C = saved;
    gate_fn(fn);
}

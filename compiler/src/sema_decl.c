// sema_decl.c - module loading, name registration, type resolution, decls.
#include <limits.h>
#include <unistd.h>

#include "sema_int.h"

Program g_prog;
FnCtx *C;

static SymMap mod_by_path;
static SymMap mod_state;  // path -> 1 loading, 2 done
static const char *g_std_dir;
static char g_cwd[PATH_MAX];

// ---------------------------------------------------------------------------
// Module loading
// ---------------------------------------------------------------------------
static bool has_ext(const char *p) {
    const char *slash = strrchr(p, '/');
    const char *dot = strrchr(p, '.');
    return dot && (!slash || dot > slash);
}

static const char *display_path(const char *real) {
    size_t n = strlen(g_cwd);
    if (n && strncmp(real, g_cwd, n) == 0 && real[n] == '/') return real + n + 1;
    return real;
}

static Module *load_module(const char *fs_path, Span *from) {
    char rp[PATH_MAX];
    if (!realpath(fs_path, rp)) {
        if (from) diag_error(*from, "E0211", "cannot find module '%s'", fs_path);
        return NULL;
    }
    Sym key = intern_cstr(rp);
    Module *m = symmap_get(&mod_by_path, key);
    if (m) {
        if ((intptr_t)symmap_get(&mod_state, key) == 1 && from)
            diag_error(*from, "E0210", "import cycle: '%s' is already being imported", display_path(rp));
        return m;
    }
    SourceFile *f = load_file(rp, arena_strndup(display_path(rp), strlen(display_path(rp))));
    if (!f) {
        if (from) diag_error(*from, "E0211", "cannot read module '%s'", fs_path);
        return NULL;
    }
    m = parse_module(f);
    m->path = key;
    symmap_put(&mod_by_path, key, m);
    symmap_put(&mod_state, key, (void *)1);
    char dir[PATH_MAX];
    snprintf(dir, sizeof dir, "%s", rp);
    char *slash = strrchr(dir, '/');
    if (slash) *slash = 0;
    for (int i = 0; i < m->decls.len; i++) {
        Decl *d = m->decls.data[i];
        if (d->kind != D_IMPORT || !d->import_path) continue;
        char path[PATH_MAX * 2];
        const char *ip = d->import_path;
        if (strncmp(ip, "std/", 4) == 0) {
            snprintf(path, sizeof path, "%s/%s%s", g_std_dir, ip + 4, has_ext(ip) ? "" : ".ph");
        } else if (ip[0] == '/') {
            snprintf(path, sizeof path, "%s%s", ip, has_ext(ip) ? "" : ".ph");
        } else {
            snprintf(path, sizeof path, "%s/%s%s", dir, ip, has_ext(ip) ? "" : ".ph");
        }
        d->imported = load_module(path, &d->import_path_span);
    }
    symmap_put(&mod_state, key, (void *)2);
    vec_push(g_prog.modules, m);
    return m;
}

// ---------------------------------------------------------------------------
// Registration
// ---------------------------------------------------------------------------
static bool is_constraint_name(Sym s) { return constraint_from_name(s) != C_NONE && (int)constraint_from_name(s) >= 0; }

static void dup_error(Span sp, Sym name, Span prev) {
    Diag *d = diag_error(sp, "E0201", "'%s' is already declared", name);
    diag_note(d, prev, "previous declaration of '%s'", name);
}

static void register_module(Module *m) {
    for (int i = 0; i < m->decls.len; i++) {
        Decl *d = m->decls.data[i];
        if (d->kind == D_ERROR) continue;
        if (!d->name) continue;
        if (is_constraint_name(d->name)) {
            diag_error(d->name_span, "E0206", "'%s' is a reserved constraint name", d->name);
            continue;
        }
        Decl *prev = symmap_get(&m->names, d->name);
        if (!prev) prev = symmap_get(&m->imports, d->name);
        if (prev) {
            dup_error(d->name_span, d->name, prev->name_span);
            continue;
        }
        if (d->kind == D_IMPORT) symmap_put(&m->imports, d->name, d);
        else symmap_put(&m->names, d->name, d);
        if (d->kind == D_STRUCT || d->kind == D_UNION) {
            for (int j = 0; j < d->methods.len; j++) {
                Decl *md = d->methods.data[j];
                md->module = m;
                Decl *pm = symmap_get(&d->method_map, md->name);
                if (pm) {
                    dup_error(md->name_span, md->name, pm->name_span);
                    continue;
                }
                symmap_put(&d->method_map, md->name, md);
            }
            for (int j = 0; j < d->fields.len; j++)
                for (int k = 0; k < j; k++)
                    if (d->fields.data[j].name == d->fields.data[k].name)
                        dup_error(d->fields.data[j].span, d->fields.data[j].name, d->fields.data[k].span);
            for (int j = 0; j < d->variants.len; j++)
                for (int k = 0; k < j; k++)
                    if (d->variants.data[j].name == d->variants.data[k].name)
                        dup_error(d->variants.data[j].span, d->variants.data[j].name, d->variants.data[k].span);
        }
    }
}

static int sym_cmp(const void *a, const void *b) { return strcmp(*(Sym const *)a, *(Sym const *)b); }

static void assign_error_codes(void) {
    SymMap seen = {0};
    Vec(Sym) names = {0};
    // Prelude: errors produced by builtins and the standard library.
    Sym prelude[] = {intern_cstr("OutOfMemory"), intern_cstr("Overflow")};
    for (size_t i = 0; i < 2; i++) {
        symmap_put(&seen, prelude[i], (void *)1);
        vec_push(names, prelude[i]);
    }
    for (int mi = 0; mi < g_prog.modules.len; mi++) {
        Module *m = g_prog.modules.data[mi];
        for (int i = 0; i < m->decls.len; i++) {
            Decl *d = m->decls.data[i];
            if (d->kind != D_ERROR) continue;
            for (int j = 0; j < d->error_names.len; j++) {
                Sym n = d->error_names.data[j];
                if (symmap_get(&seen, n)) continue;
                symmap_put(&seen, n, (void *)1);
                vec_push(names, n);
            }
        }
    }
    qsort(names.data, (size_t)names.len, sizeof(Sym), sym_cmp);
    for (int i = 0; i < names.len; i++) {
        vec_push(g_prog.error_names, names.data[i]);
        symmap_put(&g_prog.error_codes, names.data[i], (void *)(intptr_t)(i + 1));
    }
    if (names.len > 65535) diag_error(g_prog.root->decls.data[0]->span, "E0651", "too many error names");
}

uint32_t error_code_of(Sym name) { return (uint32_t)(intptr_t)symmap_get(&g_prog.error_codes, name); }

// ---------------------------------------------------------------------------
// Lookup helpers
// ---------------------------------------------------------------------------
Decl *module_lookup(Module *m, Sym name) { return m ? symmap_get(&m->names, name) : NULL; }
Decl *find_method(Decl *owner, Sym name) { return symmap_get(&owner->method_map, name); }

int find_variant(Decl *u, Sym name) {
    for (int i = 0; i < u->variants.len; i++)
        if (u->variants.data[i].name == name) return i;
    return -1;
}

int find_field(Decl *s, Sym name) {
    for (int i = 0; i < s->fields.len; i++)
        if (s->fields.data[i].name == name) return i;
    return -1;
}

static void ensure_gparams(Decl *d) {
    for (int i = 0; i < d->gparams.len; i++) {
        GenericParam *g = &d->gparams.data[i];
        if (g->type) continue;
        Constraint c = C_NONE;
        Type *ct = NULL;
        if (g->is_const) {
            TypeCtx tc = {d->module, NULL, NULL, NULL};
            ct = resolve_type(g->const_te, &tc);
            if (ct->kind != TY_INT && ct->kind != TY_INVALID) {
                diag_error(g->const_te->span, "E0471", "const generic parameter must have an integer type, found %s",
                           type_str(ct));
                ct = t_usize;
            }
        } else if (g->constraint) {
            int cc = (int)constraint_from_name(g->constraint);
            if (cc < 0 || !g->constraint) {
                diag_error(g->constraint_span, "E0470",
                           "unknown constraint '%s' (expected Eq, Ord, Num, Int, SignedInt, UnsignedInt or Float)",
                           g->constraint);
                cc = C_NONE;
            }
            c = (Constraint)cc;
        }
        g->type = type_param(d, i, g->name, c, g->is_const, ct);
        for (int j = 0; j < i; j++)
            if (d->gparams.data[j].name == g->name) dup_error(g->span, g->name, d->gparams.data[j].span);
    }
}

Type **decl_gparam_types(Decl *d) {
    ensure_gparams(d);
    Type **ts = NEWN(Type *, d->gparams.len + 1);
    for (int i = 0; i < d->gparams.len; i++) ts[i] = d->gparams.data[i].type;
    return ts;
}

Type *make_self_type(Decl *d) {
    Type **ts = decl_gparam_types(d);
    return type_named(d, ts, d->gparams.len);
}

Type *lookup_gparam(TypeCtx *tc, Sym name) {
    Decl *ds[2] = {tc->gfn, tc->gowner};
    for (int k = 0; k < 2; k++) {
        Decl *d = ds[k];
        if (!d) continue;
        ensure_gparams(d);
        for (int i = 0; i < d->gparams.len; i++)
            if (d->gparams.data[i].name == name) return d->gparams.data[i].type;
    }
    return NULL;
}

// ---------------------------------------------------------------------------
// Type resolution
// ---------------------------------------------------------------------------
static Decl *resolve_named_decl(TypeExpr *te, TypeCtx *tc, bool report) {
    Module *m = tc->mod;
    if (te->qual) {
        Decl *imp = symmap_get(&m->imports, te->qual);
        if (!imp) {
            if (report) diag_error(te->span, "E0202", "unknown module '%s'", te->qual);
            return NULL;
        }
        if (!imp->imported) return NULL;
        Decl *d = module_lookup(imp->imported, te->name);
        if (!d) {
            if (report) diag_error(te->name_span, "E0202", "module '%s' has no declaration '%s'", te->qual, te->name);
            return NULL;
        }
        if (!d->is_pub) {
            if (report) diag_error(te->name_span, "E0203", "'%s' is not public in module '%s'", te->name, te->qual);
            return NULL;
        }
        return d;
    }
    Decl *d = module_lookup(m, te->name);
    if (!d && report) diag_error(te->name_span, "E0202", "unknown type '%s'", te->name);
    return d;
}

// A generic argument: a type, an integer literal, or a const name.
static Type *resolve_generic_arg(TypeExpr *te, TypeCtx *tc) {
    if (te->kind == TE_INTLIT) return type_constint((int64_t)te->ival);
    if (te->kind == TE_NAMED && !te->has_args) {
        if (!te->qual) {
            Type *gp = lookup_gparam(tc, te->name);
            if (gp) return gp;
        }
        Decl *d = resolve_named_decl(te, tc, false);
        if (d && d->kind == D_CONST) {
            resolve_decl(d);
            if (d->cexpr && d->cexpr->is_const && d->type->kind == TY_INT) return type_constint((int64_t)d->cexpr->cval);
            diag_error(te->span, "E0472", "const '%s' is not an integer constant", te->name);
            return t_invalid;
        }
    }
    return resolve_type(te, tc);
}

bool check_generic_args(Decl *d, Type **args, int n, Span sp, TypeExprVec *tes) {
    ensure_gparams(d);
    if (n != d->gparams.len) {
        diag_error(sp, "E0467", "'%s' takes %d generic argument%s, but %d %s given", d->name, d->gparams.len,
                   d->gparams.len == 1 ? "" : "s", n, n == 1 ? "was" : "were");
        return false;
    }
    bool ok = true;
    for (int i = 0; i < n; i++) {
        GenericParam *g = &d->gparams.data[i];
        Type *a = args[i];
        Span asp = tes && i < tes->len ? tes->data[i]->span : sp;
        if (a->kind == TY_INVALID) continue;
        if (g->is_const) {
            if (a->kind == TY_CONSTINT) {
                if (!fits_type(a->cval, g->type->const_type)) {
                    diag_error(asp, "E0462", "value %lld does not fit const parameter '%s' of type %s",
                               (long long)a->cval, g->name, type_str(g->type->const_type));
                    ok = false;
                }
            } else if (!(a->kind == TY_PARAM && a->is_const && a->const_type == g->type->const_type)) {
                diag_error(asp, "E0462", "generic parameter '%s' expects a %s constant, found %s", g->name,
                           type_str(g->type->const_type), type_str(a));
                ok = false;
            }
        } else {
            if (a->kind == TY_CONSTINT || (a->kind == TY_PARAM && a->is_const)) {
                diag_error(asp, "E0462", "generic parameter '%s' expects a type, found a constant", g->name);
                ok = false;
            } else if (!type_satisfies(a, g->type->constraint)) {
                Diag *dg = diag_error(asp, "E0462", "type %s does not satisfy constraint %s of parameter '%s'",
                                      type_str(a), constraint_name(g->type->constraint), g->name);
                diag_note(dg, g->span, "'%s' declared here", g->name);
                ok = false;
            }
        }
    }
    return ok;
}

Type *resolve_type(TypeExpr *te, TypeCtx *tc) {
    if (!te) return t_invalid;
    switch (te->kind) {
    case TE_PRIM: return type_prim(te->prim);
    case TE_ERROR: return t_error;
    case TE_SELF:
        if (!tc->self) {
            diag_error(te->span, "E0205", "'Self' is only available inside a struct or union");
            return t_invalid;
        }
        return tc->self;
    case TE_INTLIT: diag_error(te->span, "E0400", "expected a type, found an integer"); return t_invalid;
    case TE_ERRUNION: {
        Type *e = resolve_type(te->elem, tc);
        if (e->kind == TY_ERRUNION) {
            diag_error(te->span, "E0401", "nested error unions (!!T) are not allowed");
            return e;
        }
        return type_errunion(e);
    }
    case TE_PTR: return type_ptr(resolve_type(te->elem, tc), te->mut);
    case TE_SLICE: {
        Type *e = resolve_type(te->elem, tc);
        if (e->kind == TY_VOID) diag_error(te->span, "E0404", "slice element type cannot be void");
        return type_slice(e, te->mut);
    }
    case TE_ARRAY: {
        Type *e = resolve_type(te->elem, tc);
        if (e->kind == TY_VOID) diag_error(te->span, "E0404", "array element type cannot be void");
        TypeExpr *l = te->len;
        if (!l) return t_invalid;
        if (l->kind == TE_INTLIT) return type_array(e, (int64_t)l->ival);
        Type *a = resolve_generic_arg(l, tc);
        if (a->kind == TY_CONSTINT) {
            if (a->cval < 0) diag_error(l->span, "E0472", "array length must not be negative");
            return type_array(e, a->cval < 0 ? 0 : a->cval);
        }
        if (a->kind == TY_PARAM && a->is_const) return type_array_param(e, a);
        if (a->kind != TY_INVALID) diag_error(l->span, "E0472", "array length must be an integer constant");
        return t_invalid;
    }
    case TE_FN: {
        Type **ps = NEWN(Type *, te->args.len + 1);
        for (int i = 0; i < te->args.len; i++) ps[i] = resolve_type(te->args.data[i], tc);
        return type_fn(ps, te->args.len, resolve_type(te->elem, tc));
    }
    case TE_ANON_STRUCT: {
        FieldVec fv = {0};
        for (int i = 0; i < te->fields.len; i++) {
            Field f = te->fields.data[i];
            f.type = resolve_type(f.te, tc);
            for (int j = 0; j < i; j++)
                if (fv.data[j].name == f.name) dup_error(f.span, f.name, fv.data[j].span);
            vec_push(fv, f);
        }
        Decl *d = anon_struct_decl(tc->mod, fv, te->span);
        return d->type;
    }
    case TE_NAMED: {
        if (!te->qual) {
            Type *gp = lookup_gparam(tc, te->name);
            if (gp) {
                if (te->has_args) diag_error(te->span, "E0468", "generic parameter '%s' cannot take arguments", te->name);
                if (gp->is_const) {
                    diag_error(te->span, "E0400", "'%s' is a constant, not a type", te->name);
                    return t_invalid;
                }
                return gp;
            }
        }
        Decl *d = resolve_named_decl(te, tc, true);
        if (!d) return t_invalid;
        if (d->kind != D_STRUCT && d->kind != D_UNION) {
            diag_error(te->name_span, "E0400", "'%s' is not a type", te->name);
            return t_invalid;
        }
        resolve_decl(d);
        if (d->gparams.len == 0) {
            if (te->has_args) diag_error(te->span, "E0468", "'%s' is not generic", te->name);
            return d->type;
        }
        if (!te->has_args) {
            Diag *dg = diag_error(te->span, "E0460", "generic type '%s' must be given arguments: %s.[...]", te->name,
                                  te->name);
            diag_note(dg, d->name_span, "'%s' declared here", d->name);
            return t_invalid;
        }
        Type **args = NEWN(Type *, te->args.len + 1);
        for (int i = 0; i < te->args.len; i++) args[i] = resolve_generic_arg(te->args.data[i], tc);
        if (!check_generic_args(d, args, te->args.len, te->span, &te->args)) return t_invalid;
        return type_named(d, args, te->args.len);
    }
    }
    return t_invalid;
}

// ---------------------------------------------------------------------------
// Declarations
// ---------------------------------------------------------------------------
static bool boundary_type_ok(Type *t, bool is_ret) {
    switch (t->kind) {
    case TY_BOOL: case TY_INT: case TY_FLOAT: case TY_PTR: case TY_FN: case TY_VEC: case TY_V128: case TY_INVALID:
        return true;
    case TY_VOID: return is_ret;
    default: return false;
    }
}

static void resolve_fn(Decl *d) {
    Decl *owner = d->owner;
    if (owner) resolve_decl(owner);
    ensure_gparams(d);
    int nown = owner ? owner->gparams.len : 0;
    d->nall_gparams = nown + d->gparams.len;
    d->all_gparams = NEWN(Type *, d->nall_gparams + 1);
    for (int i = 0; i < nown; i++) d->all_gparams[i] = owner->gparams.data[i].type;
    for (int i = 0; i < d->gparams.len; i++) d->all_gparams[nown + i] = d->gparams.data[i].type;
    TypeCtx tc = {d->module, owner, d, owner ? owner->type : NULL};
    d->param_types = NEWN(Type *, d->params.len + 1);
    for (int i = 0; i < d->params.len; i++) {
        Param *p = &d->params.data[i];
        p->local = NULL;
        d->param_types[i] = resolve_type(p->te, &tc);
        if (d->param_types[i]->kind == TY_VOID) {
            diag_error(p->te->span, "E0404", "parameter '%s' cannot have type void", p->name);
            d->param_types[i] = t_invalid;
        }
        for (int j = 0; j < i; j++)
            if (d->params.data[j].name == p->name) dup_error(p->span, p->name, d->params.data[j].span);
    }
    d->ret_type = d->ret_te ? resolve_type(d->ret_te, &tc) : t_void;
    d->type = type_fn(d->param_types, d->params.len, d->ret_type);
    d->self_kind = SELF_NONE;
    if (owner && d->params.len > 0 && strcmp(d->params.data[0].name, "self") == 0) {
        Type *pt = d->param_types[0];
        Type *self = owner->type;
        if (pt == self) d->self_kind = SELF_VALUE;
        else if (pt->kind == TY_PTR && pt->elem == self) d->self_kind = pt->mut ? SELF_PASS : SELF_ADDR;
        else if (pt->kind != TY_INVALID)
            diag_error(d->params.data[0].te->span, "E0305", "'self' must have type Self, *Self or *mut Self, found %s",
                       type_str(pt));
    }
    if (d->extern_c && d->abi && strcmp(d->abi, "C") != 0)
        diag_error(d->abi_span, "E0306", "unknown ABI \"%s\" (only \"C\" is supported)", d->abi);
    if (d->variadic && !d->extern_c)
        diag_error(d->abi_span, "E0307", "'...' (C varargs) is only allowed in extern \"C\" declarations");
    else if (d->variadic && d->params.len == 0)
        diag_error(d->abi_span, "E0307", "a variadic C function needs at least one fixed parameter");
    if (d->is_export || d->kind == D_HOST_FN) {
        const char *what = d->is_export ? "exported" : d->extern_c ? "C" : "imported";
        if (d->gparams.len) diag_error(d->name_span, "E0466", "%s functions cannot be generic", what);
        for (int i = 0; i < d->params.len; i++)
            if (!boundary_type_ok(d->param_types[i], false))
                diag_error(d->params.data[i].te->span, "E0801", "type %s is not allowed at the %s function boundary",
                           type_str(d->param_types[i]), what);
        if (!boundary_type_ok(d->ret_type, true))
            diag_error(d->ret_te ? d->ret_te->span : d->name_span, "E0801",
                       "return type %s is not allowed at the %s function boundary%s", type_str(d->ret_type), what,
                       d->ret_type->kind == TY_ERRUNION ? " (handle errors before the export boundary)" : "");
    }
}

void resolve_decl(Decl *d) {
    if (d->state == 2) return;
    if (d->state == 1) {
        if (d->kind == D_CONST)
            diag_error(d->name_span, "E0351", "constant '%s' depends on itself", d->name);
        return;
    }
    d->state = 1;
    switch (d->kind) {
    case D_STRUCT:
    case D_UNION: {
        d->type = make_self_type(d);
        TypeCtx tc = {d->module, d, NULL, d->type};
        d->state = 2;  // fields may refer to Self through pointers
        for (int i = 0; i < d->fields.len; i++) {
            Field *f = &d->fields.data[i];
            f->type = resolve_type(f->te, &tc);
            if (f->type->kind == TY_VOID) {
                diag_error(f->te->span, "E0404", "field '%s' cannot have type void", f->name);
                f->type = t_invalid;
            }
        }
        for (int i = 0; i < d->variants.len; i++) {
            Variant *v = &d->variants.data[i];
            if (v->te) {
                v->type = resolve_type(v->te, &tc);
                if (v->type->kind == TY_VOID) v->type = NULL;
            }
        }
        if (d->kind == D_UNION && d->variants.len == 0)
            diag_error(d->name_span, "E0304", "union '%s' must have at least one variant", d->name);
        if (d->kind == D_UNION && d->variants.len > 65536)
            diag_error(d->name_span, "E0304", "union '%s' has too many variants", d->name);
        return;
    }
    case D_FN:
    case D_HOST_FN: resolve_fn(d); break;
    case D_CONST: {
        TypeCtx tc = {d->module, NULL, NULL, NULL};
        d->type = resolve_type(d->te, &tc);
        if (d->type->kind != TY_INT && d->type->kind != TY_FLOAT && d->type->kind != TY_BOOL &&
            d->type->kind != TY_INVALID) {
            diag_error(d->te->span, "E0352", "constants must have an integer, float or bool type, found %s",
                       type_str(d->type));
            d->type = t_invalid;
        }
        FnCtx ctx = {.tc = tc, .const_ctx = true};
        FnCtx *saved = C;
        C = &ctx;
        check_expr_to(&d->init, d->type);
        C = saved;
        d->cexpr = d->init;
        if (!d->init->is_const && d->init->kind != E_BOOL && d->type->kind != TY_INVALID &&
            d->init->type->kind != TY_INVALID)
            diag_error(d->init->span, "E0350", "constant initializer must be a constant expression");
        break;
    }
    case D_GLOBAL: {
        TypeCtx tc = {d->module, NULL, NULL, NULL};
        d->type = resolve_type(d->te, &tc);
        if (d->type->kind == TY_VOID) {
            diag_error(d->te->span, "E0404", "global '%s' cannot have type void", d->name);
            d->type = t_invalid;
        }
        break;
    }
    default: break;
    }
    d->state = 2;
}

// ---------------------------------------------------------------------------
// Infinite-size detection
// ---------------------------------------------------------------------------
static bool contains_by_value(Type *t, Decl *target, int depth) {
    if (depth > 64) return false;
    switch (t->kind) {
    case TY_ARRAY: return contains_by_value(t->elem, target, depth + 1);
    case TY_ERRUNION: return contains_by_value(t->elem, target, depth + 1);
    case TY_STRUCT:
        if (t->decl == target && depth > 0) return true;
        for (int i = 0; i < t->decl->fields.len; i++)
            if (contains_by_value(type_field(t, i), target, depth + 1)) return true;
        return false;
    case TY_UNION:
        if (t->decl == target && depth > 0) return true;
        for (int i = 0; i < t->decl->variants.len; i++) {
            Type *p = type_variant(t, i);
            if (p && contains_by_value(p, target, depth + 1)) return true;
        }
        return false;
    default: return false;
    }
}

// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------
static bool is_const_init(Expr *e) {
    if (e->is_const) return true;
    switch (e->kind) {
    case E_INT: case E_FLOAT: case E_BOOL: case E_STRING: case E_ERROR_VAL: return true;
    case E_CONV: return is_const_init(e->a);
    case E_ARRAY_LIT:
        for (int i = 0; i < e->args.len; i++)
            if (!is_const_init(e->args.data[i])) return false;
        return true;
    case E_ARRAY_REPEAT: return is_const_init(e->a);
    case E_STRUCT_LIT:
        for (int i = 0; i < e->inits.len; i++)
            if (!is_const_init(e->inits.data[i].value)) return false;
        return true;
    case E_FIELD:
        return e->op == FK_VARIANT_UNIT || e->op == FK_CONST || (e->op == FK_DECL && e->decl && e->decl->kind == D_FN);
    case E_IDENT: return e->cat == CAT_VALUE && e->decl && e->decl->kind == D_FN && e->type->kind == TY_FN;
    case E_CALL: return e->op == CALLK_VARIANT && is_const_init(e->args.data[0]);
    case E_BUILTIN: return e->index == BI_PTR_FROM_INT && is_const_init(e->bargs.data[1].e);
    case E_UNARY: return e->op == TK_MINUS && is_const_init(e->a) && e->a->kind == E_FLOAT;
    default: return false;
    }
}

static void check_global(Decl *d) {
    resolve_decl(d);
    FnCtx ctx = {.tc = {d->module, NULL, NULL, NULL}, .const_ctx = true};
    FnCtx *saved = C;
    C = &ctx;
    check_expr_to(&d->init, d->type);
    C = saved;
    if (d->type->kind != TY_INVALID && d->init->type->kind != TY_INVALID && !is_const_init(d->init))
        diag_error(d->init->span, "E0350", "global initializer must be a constant expression");
}

// ---------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------
bool sema_run(const char *path, SemaOptions *opts) {
    g_std_dir = opts->std_dir;
    if (!getcwd(g_cwd, sizeof g_cwd)) g_cwd[0] = 0;
    Module *root = load_module(path, NULL);
    if (!root) return false;
    g_prog.root = root;
    for (int i = 0; i < g_prog.modules.len; i++) register_module(g_prog.modules.data[i]);
    assign_error_codes();

    // Resolve all declarations.
    for (int mi = 0; mi < g_prog.modules.len; mi++) {
        Module *m = g_prog.modules.data[mi];
        for (int i = 0; i < m->decls.len; i++) {
            Decl *d = m->decls.data[i];
            if (d->kind == D_IMPORT || d->kind == D_ERROR) continue;
            resolve_decl(d);
            for (int j = 0; j < d->methods.len; j++) resolve_decl(d->methods.data[j]);
        }
    }
    // Infinite-size types.
    for (int mi = 0; mi < g_prog.modules.len; mi++) {
        Module *m = g_prog.modules.data[mi];
        for (int i = 0; i < m->decls.len; i++) {
            Decl *d = m->decls.data[i];
            if ((d->kind == D_STRUCT || d->kind == D_UNION) && contains_by_value(d->type, d, 0))
                diag_error(d->name_span, "E0403", "'%s' contains itself by value and would have infinite size", d->name);
        }
    }
    // Exported names must be unique program-wide.
    SymMap exports = {0};
    for (int mi = 0; mi < g_prog.modules.len; mi++) {
        Module *m = g_prog.modules.data[mi];
        for (int i = 0; i < m->decls.len; i++) {
            Decl *d = m->decls.data[i];
            if (d->kind == D_FN && d->is_export) {
                Decl *p = symmap_get(&exports, d->name);
                if (p) {
                    Diag *dg = diag_error(d->name_span, "E0802", "export name '%s' is used more than once", d->name);
                    diag_note(dg, p->name_span, "first exported here");
                } else {
                    symmap_put(&exports, d->name, d);
                }
            }
        }
    }
    // Globals and function bodies.
    for (int mi = 0; mi < g_prog.modules.len; mi++) {
        Module *m = g_prog.modules.data[mi];
        for (int i = 0; i < m->decls.len; i++) {
            Decl *d = m->decls.data[i];
            switch (d->kind) {
            case D_GLOBAL:
                check_global(d);
                vec_push(g_prog.globals, d);
                break;
            case D_FN:
                check_fn_body(d);
                vec_push(g_prog.fns, d);
                break;
            case D_HOST_FN: vec_push(g_prog.host_fns, d); break;
            case D_STRUCT:
            case D_UNION:
                for (int j = 0; j < d->methods.len; j++) {
                    check_fn_body(d->methods.data[j]);
                    vec_push(g_prog.fns, d->methods.data[j]);
                }
                break;
            default: break;
            }
        }
    }
    return diag_error_count() == 0;
}

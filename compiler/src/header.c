// header.c - C header for a program's exported functions (spec §14.5.4).
//
// Every `export fn` gets a prototype. Structs reachable from the exported
// signatures are defined with the same layout Axiom uses (spec §5.9 is
// C's natural layout), so C code can read and write them through pointers.
#include <ctype.h>

#include "header.h"

typedef Vec(Type *) TypeVec;

typedef struct HeaderGen {
    TypeVec emitted;   // structs already defined
    TypeVec visiting;  // cycle guard
    CharVec defs;          // struct definitions, in dependency order
} HeaderGen;

static void cat(CharVec *b, const char *s) {
    while (*s) vec_push(*b, *s++);
}

static const char *sanitize(const char *s) {
    CharVec b = {0};
    for (; *s; s++) vec_push(b, (isalnum((unsigned char)*s) || *s == '_') ? *s : '_');
    // collapse runs of '_' and trim trailing ones
    CharVec c = {0};
    for (int i = 0; i < b.len; i++)
        if (!(b.data[i] == '_' && c.len && vec_last(c) == '_')) vec_push(c, b.data[i]);
    while (c.len && vec_last(c) == '_') c.len--;
    vec_push(c, 0);
    char *r = arena_strndup(c.data, strlen(c.data));
    free(b.data);
    free(c.data);
    return r;
}

static const char *struct_c_name(Type *t) {
    if (t->decl->anon) return arena_printf("axiom_anon_%d", t->id);
    return sanitize(type_str(t));
}

static bool contains(TypeVec *v, Type *t) {
    for (int i = 0; i < v->len; i++)
        if (v->data[i] == t) return true;
    return false;
}

static void need_struct(HeaderGen *g, Type *t);

// Base type name for scalars, or NULL.
static const char *scalar_name(Type *t) {
    switch (t->kind) {
    case TY_VOID: return "void";
    case TY_BOOL: return "bool";
    case TY_INT:
        if (t->is_size) return t->is_signed ? "ptrdiff_t" : "size_t";
        switch (t->bits) {
        case 8: return t->is_signed ? "int8_t" : "uint8_t";
        case 16: return t->is_signed ? "int16_t" : "uint16_t";
        case 32: return t->is_signed ? "int32_t" : "uint32_t";
        default: return t->is_signed ? "int64_t" : "uint64_t";
        }
    case TY_FLOAT: return t->bits == 32 ? "float" : "double";
    case TY_ERROR: return "uint16_t";
    default: return NULL;
    }
}

// C declaration of `inner` with type t. `konst`: the declared object is const.
static const char *decl(HeaderGen *g, Type *t, const char *inner, bool konst) {
    const char *k = konst ? "const " : "";
    const char *sep = inner[0] ? " " : "";
    const char *sc = scalar_name(t);
    if (sc) return arena_printf("%s%s%s%s", k, sc, sep, inner);
    switch (t->kind) {
    case TY_PTR: {
        const char *in = arena_printf("*%s%s", konst ? "const " : "", inner);
        if (t->elem->kind == TY_ARRAY) in = arena_printf("(%s)", in);
        Type *e = t->elem;
        if (e->kind == TY_STRUCT || scalar_name(e) || e->kind == TY_PTR || e->kind == TY_FN || e->kind == TY_ARRAY)
            return decl(g, e, in, !t->mut);
        return arena_printf("%svoid %s", t->mut ? "" : "const ", in);  // pointee has no C spelling
    }
    case TY_FN: {
        CharVec ps = {0};
        if (t->nparams == 0) cat(&ps, "void");
        for (int i = 0; i < t->nparams; i++) {
            if (i) cat(&ps, ", ");
            cat(&ps, decl(g, t->params[i], "", false));
        }
        vec_push(ps, 0);
        const char *r = decl(g, t->ret, arena_printf("(*%s%s)(%s)", konst ? "const " : "", inner, ps.data), false);
        free(ps.data);
        return r;
    }
    case TY_ARRAY: return decl(g, t->elem, arena_printf("%s[%lld]", inner, (long long)t->len), konst);
    case TY_STRUCT:
        need_struct(g, t);
        return arena_printf("%s%s%s%s", k, struct_c_name(t), sep, inner);
    case TY_SLICE: {
        const char *el = decl(g, t->elem, "*ptr", !t->mut);
        if (!scalar_name(t->elem) && t->elem->kind != TY_STRUCT && t->elem->kind != TY_PTR)
            el = arena_printf("%svoid *ptr", t->mut ? "" : "const ");
        return arena_printf("%sstruct { %s; size_t len; }%s%s", k, el, sep, inner);
    }
    case TY_VEC:
    case TY_V128: {
        const char *lane = t->kind == TY_V128 ? "int32_t" : scalar_name(t->elem);
        return arena_printf("%s%s __attribute__((vector_size(16)))%s%s", k, lane, sep, inner);
    }
    default:
        // Tagged unions and error unions have no C equivalent: an opaque blob
        // with the right size and alignment keeps enclosing layouts correct.
        return arena_printf("%sAXIOM_ALIGNAS(%lld) unsigned char %s[%lld]", k, (long long)type_align(t), inner,
                            (long long)type_size(t));
    }
}

static void need_struct(HeaderGen *g, Type *t) {
    if (contains(&g->emitted, t)) return;
    const char *name = struct_c_name(t);
    if (contains(&g->visiting, t)) return;  // recursion through pointers: typedef already emitted
    vec_push(g->visiting, t);
    // forward typedef first, so self-referential pointers work
    cat(&g->defs, arena_printf("typedef struct %s %s;\n", name, name));
    CharVec body = {0};
    cat(&body, arena_printf("struct %s {\n", name));
    for (int i = 0; i < t->decl->fields.len; i++) {
        Type *ft = type_field(t, i);
        cat(&body, "    ");
        cat(&body, decl(g, ft, t->decl->fields.data[i].name, false));
        cat(&body, ";\n");
    }
    if (t->decl->fields.len == 0) cat(&body, "    char axiom_empty[1]; /* C forbids empty structs; size differs */\n");
    cat(&body, arena_printf("};\nAXIOM_STATIC_ASSERT(sizeof(struct %s) == %lld, \"layout mismatch\");\n\n", name,
                            (long long)type_size(t)));
    vec_push(body, 0);
    cat(&g->defs, body.data);
    free(body.data);
    g->visiting.len--;
    vec_push(g->emitted, t);
}

bool emit_c_header(const char *path, const char *source_name) {
    HeaderGen g = {0};
    CharVec protos = {0};
    for (int i = 0; i < g_prog.fns.len; i++) {
        Decl *d = g_prog.fns.data[i];
        if (!d->is_export) continue;
        CharVec ps = {0};
        if (d->params.len == 0) cat(&ps, "void");
        for (int j = 0; j < d->params.len; j++) {
            if (j) cat(&ps, ", ");
            cat(&ps, decl(&g, d->param_types[j], d->params.data[j].name, false));
        }
        vec_push(ps, 0);
        cat(&protos, decl(&g, d->ret_type, arena_printf("%s(%s)", d->name, ps.data), false));
        cat(&protos, ";\n");
        free(ps.data);
    }
    vec_push(protos, 0);
    vec_push(g.defs, 0);
    FILE *f = fopen(path, "w");
    if (!f) return false;
    const char *guard = arena_printf("AXIOM_%s_H", sanitize(source_name));
    for (char *p = (char *)guard; *p; p++) *p = (char)toupper((unsigned char)*p);
    fprintf(f,
            "/* Generated by axiom from %s for target %s. Do not edit.\n"
            "   Struct layouts depend on the target: regenerate the header per target. */\n"
            "#ifndef %s\n#define %s\n\n#include <stdbool.h>\n#include <stddef.h>\n#include <stdint.h>\n\n"
            "#ifdef __cplusplus\n#define AXIOM_STATIC_ASSERT static_assert\n#define AXIOM_ALIGNAS(n) alignas(n)\n"
            "extern \"C\" {\n#else\n#define AXIOM_STATIC_ASSERT _Static_assert\n#define AXIOM_ALIGNAS(n) _Alignas(n)\n"
            "#endif\n\n"
            "AXIOM_STATIC_ASSERT(sizeof(void *) == %d, \"this header was generated for %d-bit pointers\");\n\n"
            "%s%s\n#ifdef __cplusplus\n}\n#endif\n\n#endif\n",
            source_name, g_target.triple, guard, guard, g_target.ptr_bits / 8, g_target.ptr_bits, g.defs.data, protos.data);
    fclose(f);
    free(protos.data);
    free(g.defs.data);
    return true;
}

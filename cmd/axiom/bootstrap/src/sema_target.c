// sema_target.c - processor extensions (spec §13.4): target conditions,
// @features and @multiversion, and builtins that need an extension.
#include "sema_int.h"
#include "target.h"

#include <string.h>

// The value a target name stands for: an architecture (@target_arch) or an
// operating system (@target_os). -1 for an unknown name.
int target_name_value(int id, const char *name) {
    if (id == BI_TARGET_ARCH) {
        static const char *const t[] = {"wasm32", "wasm64", "x86_64", "aarch64"};
        for (int i = 0; i < 4; i++)
            if (!strcmp(t[i], name)) return i + ARCH_WASM32;
        return -1;
    }
    static const char *const t[] = {"linux", "macos", "windows", "none"};
    for (int i = 0; i < 4; i++)
        if (!strcmp(t[i], name)) return i;
    return -1;
}

// The operating system as @target_os numbers it (none: WebAssembly).
static int target_os_value(void) {
    if (g_arch == ARCH_WASM32 || g_arch == ARCH_WASM64) return 3;
    if (g_os == OS_DARWIN) return 1;
    if (g_os == OS_WINDOWS) return 2;
    return 0;
}

int target_cond(Expr *e, uint64_t feats) {
    if (e->kind == E_BOOL) return e->bval ? 1 : 0;
    if (e->kind == E_BUILTIN) {
        int id = e->index;
        if (e->bargs.len != 1 || !e->bargs.data[0].e || e->bargs.data[0].e->kind != E_STRING) return -1;
        const char *name = e->bargs.data[0].e->str;
        if (id == BI_TARGET_HAS || id == BI_CPU_HAS) {
            uint64_t b = feature_bit(name);
            bool on = b != 0 && (feats & b) == b;
            if (id == BI_CPU_HAS && !on) return -1;
            return on ? 1 : 0;
        }
        if (id == BI_TARGET_ARCH) return target_name_value(id, name) == g_arch ? 1 : 0;
        if (id == BI_TARGET_OS) return target_name_value(id, name) == target_os_value() ? 1 : 0;
        return -1;
    }
    if (e->kind == E_UNARY && e->op == KW_NOT) {
        int a = target_cond(e->a, feats);
        return a < 0 ? -1 : 1 - a;
    }
    if (e->kind == E_BINARY && (e->op == KW_AND || e->op == KW_OR)) {
        int a = target_cond(e->a, feats), b = target_cond(e->b, feats);
        if (e->op == KW_AND) {
            if (a == 0 || b == 0) return 0;
            if (a == 1 && b == 1) return 1;
            return -1;
        }
        if (a == 1 || b == 1) return 1;
        if (a == 0 && b == 0) return 0;
        return -1;
    }
    return -1;
}

bool baseline_fn(Decl *d) {
    if (!d->module) return false;
    const char *f = g_files.data[d->module->file->id]->display;
    size_t n = strlen(f);
    return n >= 13 && !strcmp(f + n - 13, "std/os/cpu.ax");
}

// The extensions a function's code may use, without an attribute.
uint64_t fn_base_features(Decl *d) { return baseline_fn(d) ? 0 : g_features; }

void check_fn_attr(Decl *f) {
    if (f->attr == ATTR_NONE) return;
    int n = f->attr_args.len;
    f->feat_sets = NEWN(uint64_t, n + 1);
    const char *what = f->attr == ATTR_FEATURES ? "features" : "multiversion";
    if (n == 0) {
        diag_error(f->attr_span, "E0829", "@%s needs at least one set of extensions, such as \"avx2,fma\"", what);
        return;
    }
    if (f->attr == ATTR_FEATURES && n != 1)
        diag_error(f->attr_span, "E0829", "@features takes one set of extensions (\"a,b\"); @multiversion takes several");
    for (int i = 0; i < n; i++) {
        const char *spec = f->attr_args.data[i];
        Span sp = f->attr_arg_spans.data[i];
        bool empty = true;
        const char *p = spec;
        for (;;) {
            const char *q = strchr(p, ',');
            size_t len = q ? (size_t)(q - p) : strlen(p);
            char item[64];
            snprintf(item, sizeof item, "%.*s", (int)len, p);
            const char *name = item;
            while (*name == ' ') name++;
            if (*name == '+') name++;
            if (*name) {
                empty = false;
                if (!feature_known(name))
                    diag_error(sp, "E0827", "unknown processor extension '%s' (this target has: %s)", name, feature_names());
            }
            if (!q) break;
            p = q + 1;
        }
        if (empty) diag_error(sp, "E0829", "empty set of extensions");
        bool foreign;
        uint64_t bits = parse_feature_set(spec, &foreign);
        f->feat_sets[i] = foreign ? SEMA_NO_VERSION : bits;
    }
}

int fn_versions(Decl *f, uint64_t *out) {
    uint64_t base = fn_base_features(f);
    if (f->attr == ATTR_NONE) {
        out[0] = base;
        return 1;
    }
    int n = 0;
    if (f->attr == ATTR_MULTIVERSION) out[n++] = base;
    for (int i = 0; i < f->attr_args.len; i++)
        if (f->feat_sets[i] != SEMA_NO_VERSION) out[n++] = base | f->feat_sets[i];
    return n;
}

typedef struct Need {
    int arch;
    uint64_t feats;
} Need;

static Need builtin_need(int id) {
    if (id == BI_X86_CPUID || id == BI_X86_XGETBV) return (Need){ARCH_X64, 0};
    if (id >= BI_X86_AESENC && id <= BI_X86_AESKEYGENASSIST) return (Need){ARCH_X64, (uint64_t)1 << 12};  // aes
    if (id == BI_X86_CLMUL) return (Need){ARCH_X64, (uint64_t)1 << 13};                                     // pclmul
    if (id >= BI_X86_SHA1RNDS4 && id <= BI_X86_SHA256MSG2) return (Need){ARCH_X64, (uint64_t)1 << 14};      // sha
    if (id == BI_X86_CRC32C) return (Need){ARCH_X64, (uint64_t)1 << 3};                                     // sse4.2
    if (id >= BI_ARM_AESE && id <= BI_ARM_PMULL2) return (Need){ARCH_A64, (uint64_t)1 << 4};                // aes
    if (id >= BI_ARM_SHA1C && id <= BI_ARM_SHA256SU1) return (Need){ARCH_A64, (uint64_t)1 << 5};            // sha2
    if (id >= BI_ARM_SHA512H && id <= BI_ARM_SHA512SU1) return (Need){ARCH_A64, (uint64_t)1 << 7};          // sha512
    if (id >= BI_ARM_EOR3 && id <= BI_ARM_XAR) return (Need){ARCH_A64, (uint64_t)1 << 6};                   // sha3
    if (id == BI_ARM_CRC32 || id == BI_ARM_CRC32C) return (Need){ARCH_A64, (uint64_t)1 << 9};               // crc
    return (Need){0, 0};
}

static const char *arch_name(int a) {
    static const char *const t[] = {"wasm32", "wasm64", "x86_64", "aarch64"};
    return t[a - ARCH_WASM32];
}

static const char *need_text(Need nd, uint64_t feats) {
    if (nd.arch != g_arch) {
        int saved = g_arch;
        g_arch = nd.arch;
        const char *t = arena_printf("the %s architecture", arch_name(nd.arch));
        if (nd.feats) t = arena_printf("%s with %s", t, features_string(nd.feats));
        g_arch = saved;
        return t;
    }
    return arena_printf("the extensions %s", features_string(nd.feats & ~feats));
}

static Vec(uint32_t) gate_reported;

static void gate_stmt(Stmt *s, uint64_t feats);

static void gate_expr(Expr *e, uint64_t feats) {
    if (!e) return;
    if (e->kind == E_BUILTIN) {
        Need nd = builtin_need(e->index);
        if ((nd.arch != 0 && nd.arch != g_arch) || (feats & nd.feats) != nd.feats) {
            bool seen = false;
            for (int i = 0; i < gate_reported.len; i++)
                if (gate_reported.data[i] == e->span.start) seen = true;
            if (!seen) {
                vec_push(gate_reported, e->span.start);
                Diag *d = diag_error(e->span, "E0828", "@%s needs %s, which this code is not compiled for", e->name,
                                     need_text(nd, feats));
                diag_note(d, e->span,
                          "use it under `if @target_has(...)` or `if @target_arch(...)`, or in a function declared with @features or @multiversion");
            }
        }
        for (int i = 0; i < e->bargs.len; i++) gate_expr(e->bargs.data[i].e, feats);
        return;
    }
    gate_expr(e->a, feats);
    // A method callee keeps its receiver in b as well as in a, and the call (or
    // generic application) over it copies that b; walking those again would visit
    // a receiver chain exponentially often.
    if (e->b != e->a && !(e->a && e->b == e->a->b)) gate_expr(e->b, feats);
    gate_expr(e->c, feats);
    for (int i = 0; i < e->args.len; i++) gate_expr(e->args.data[i], feats);
    for (int i = 0; i < e->inits.len; i++) gate_expr(e->inits.data[i].value, feats);
    gate_expr(e->hexpr, feats);
    gate_stmt(e->hblock, feats);
}

static void gate_stmt(Stmt *s, uint64_t feats) {
    if (!s) return;
    if (s->kind == S_IF) {
        int c = target_cond(s->e, feats);
        if (c == 1) {
            gate_stmt(s->body, feats);
            return;
        }
        if (c == 0) {
            gate_stmt(s->els, feats);
            return;
        }
    }
    gate_expr(s->e, feats);
    gate_expr(s->e2, feats);
    gate_expr(s->e3, feats);
    gate_stmt(s->body, feats);
    gate_stmt(s->els, feats);
    for (int i = 0; i < s->stmts.len; i++) gate_stmt(s->stmts.data[i], feats);
    for (int i = 0; i < s->arms.len; i++)
        if (!s->arms.data[i].alt) gate_stmt(s->arms.data[i].body, feats);
}

void gate_fn(Decl *f) {
    if (!f->body) return;
    uint64_t *vs = NEWN(uint64_t, f->attr_args.len + 2);
    int n = fn_versions(f, vs);
    for (int i = 0; i < n; i++) gate_stmt(f->body, vs[i]);
}

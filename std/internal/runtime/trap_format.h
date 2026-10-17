// std/internal/runtime/trap_format.h - formatting of Axiom trap records (shared by
// native_host.c and test_main.c). See codegen: axiom.trap_info.
#ifndef AXIOM_TRAP_FORMAT_H
#define AXIOM_TRAP_FORMAT_H

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// A site record: a trap site, a call site (code 255) or an error return
// (code 254). `fn` is the enclosing function, `callee` the called one.
struct ax_trap_site {
    uint32_t code, file_id, start, end, line, col;
    const char *file, *text, *fn, *callee;
};

// A thread's shadow stack (spec §16.4); see codegen: ss_ptr.
#define AX_SS_CAP 256
#define AX_ET_CAP 32
struct ax_shadow_stack {
    uint32_t depth, errors;
    const struct ax_trap_site *calls[AX_SS_CAP];
    const struct ax_trap_site *error_returns[AX_ET_CAP];
};

struct ax_trap_info {
    uint64_t site, kind, a, b, alen, blen;
};

static const char *const ax_trap_names[] = {
    "trap", "index out of bounds", "integer overflow", "division by zero", "int_cast out of range",
    "memory access out of bounds", "length precondition failed", "invalid union tag or enum value", "stack exhausted",
    "invalid function pointer", "misaligned atomic access", "expectation failed", "memory error",
    "processor lacks a required extension",
};

static inline void ax_format_value(char *out, size_t cap, uint64_t kind, uint64_t v, uint64_t len) {
    double d;
    switch (kind) {
    case 1: snprintf(out, cap, "%" PRId64, (int64_t)v); break;
    case 2: snprintf(out, cap, "%" PRIu64, v); break;
    case 3: memcpy(&d, &v, 8); snprintf(out, cap, "%.17g", d); break;
    case 4: snprintf(out, cap, "%s", v ? "true" : "false"); break;
    case 5: snprintf(out, cap, "\"%.*s\"", (int)(len > 200 ? 200 : len), (const char *)(uintptr_t)v); break;
    case 6: snprintf(out, cap, "error #%" PRIu64, v); break;
    default: snprintf(out, cap, "0x%" PRIx64, v); break;
    }
}

// "trap T0001 (index out of bounds): <text>" or
// "expected equal values, left: 3, right: 4: <text>"
static inline void ax_format_trap(char *out, size_t cap, const struct ax_trap_site *s, const struct ax_trap_info *info) {
    const char *name = s->code < sizeof ax_trap_names / sizeof *ax_trap_names ? ax_trap_names[s->code] : "trap";
    if (info && info->kind) {
        char a[256], b[256];
        ax_format_value(a, sizeof a, info->kind, info->a, info->alen);
        ax_format_value(b, sizeof b, info->kind, info->b, info->blen);
        snprintf(out, cap, "expected equal values, left: %s, right: %s: %s", a, b, s->text);
    } else if (s->code == 11) {
        snprintf(out, cap, "expectation failed: %s", s->text);
    } else {
        snprintf(out, cap, "trap T%04u (%s): %s", s->code, name, s->text);
    }
}

// Appends "stack trace" lines for a trap at `top` (may be NULL: location
// unknown) to out. Returns the length written.
static inline size_t ax_format_stack(char *out, size_t cap, const struct ax_trap_site *top, const struct ax_shadow_stack *ss) {
    size_t n = 0;
#define AX_PUT(...)                                                               \
    do {                                                                          \
        if (n + 1 < cap) {                                                        \
            int k_ = snprintf(out + n, cap - n, __VA_ARGS__);                     \
            if (k_ > 0) n += (size_t)k_ < cap - n ? (size_t)k_ : cap - n - 1;     \
        }                                                                         \
    } while (0)
    if (cap) out[0] = 0;
    uint32_t depth = ss ? ss->depth : 0;
    if (!top && !depth) return 0;
    AX_PUT("stack trace (innermost first):\n");
    if (top) AX_PUT("  at %s (%s:%u:%u)\n", top->fn, top->file, top->line, top->col);
    else {
        const struct ax_trap_site *c = ss->calls[(depth - 1) & (AX_SS_CAP - 1)];
        AX_PUT("  at %s\n", c && c->callee[0] ? c->callee : "<indirect call>");
    }
    int shown = 0;
    for (int64_t i = (int64_t)depth - 1; i >= 0 && i >= (int64_t)depth - AX_SS_CAP; i--) {
        const struct ax_trap_site *c = ss->calls[i & (AX_SS_CAP - 1)];
        if (!c) continue;
        if (shown >= 40) {
            AX_PUT("  ... %lld more frames\n", (long long)i + 1);
            return n;
        }
        int64_t j = i;
        while (j - 1 >= 0 && j - 1 >= (int64_t)depth - AX_SS_CAP && ss->calls[(j - 1) & (AX_SS_CAP - 1)] == c) j--;
        AX_PUT("  at %s (%s:%u:%u)\n", c->fn, c->file, c->line, c->col);
        shown++;
        if (j < i) {
            AX_PUT("  ... the frame above repeats %lld more time%s\n", (long long)(i - j), i - j == 1 ? "" : "s");
            i = j;
        }
    }
    if (depth > AX_SS_CAP) AX_PUT("  ... %u more frames\n", depth - AX_SS_CAP);
    return n;
}

static inline size_t ax_format_error_trace(char *out, size_t cap, const struct ax_shadow_stack *ss) {
    size_t n = 0;
    if (cap) out[0] = 0;
    if (!ss || !ss->errors) return 0;
    AX_PUT("error return trace (oldest first):\n");
    uint32_t first = ss->errors > AX_ET_CAP ? ss->errors - AX_ET_CAP : 0;
    for (uint32_t i = first; i < ss->errors; i++) {
        const struct ax_trap_site *e = ss->error_returns[i & (AX_ET_CAP - 1)];
        if (e) AX_PUT("  %s:%u:%u in %s: %s\n", e->file, e->line, e->col, e->fn, e->text);
    }
#undef AX_PUT
    return n;
}

#endif

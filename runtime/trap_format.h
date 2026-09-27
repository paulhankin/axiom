// runtime/trap_format.h - formatting of Phalange trap records (shared by
// native_host.c and test_main.c). See codegen: phalange.trap_info.
#ifndef PHALANGE_TRAP_FORMAT_H
#define PHALANGE_TRAP_FORMAT_H

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct ph_trap_site {
    uint32_t code, file_id, start, end, line, col;
    const char *file, *text;
};

struct ph_trap_info {
    uint64_t site, kind, a, b, alen, blen;
};

static const char *const ph_trap_names[] = {
    "trap", "index out of bounds", "integer overflow", "division by zero", "int_cast out of range",
    "memory access out of bounds", "length precondition failed", "invalid union tag", "stack exhausted",
    "invalid function pointer", "misaligned atomic access", "expectation failed",
};

static void ph_format_value(char *out, size_t cap, uint64_t kind, uint64_t v, uint64_t len) {
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
static void ph_format_trap(char *out, size_t cap, const struct ph_trap_site *s, const struct ph_trap_info *info) {
    const char *name = s->code < sizeof ph_trap_names / sizeof *ph_trap_names ? ph_trap_names[s->code] : "trap";
    if (info && info->kind) {
        char a[256], b[256];
        ph_format_value(a, sizeof a, info->kind, info->a, info->alen);
        ph_format_value(b, sizeof b, info->kind, info->b, info->blen);
        snprintf(out, cap, "expected equal values, left: %s, right: %s: %s", a, b, s->text);
    } else if (s->code == 11) {
        snprintf(out, cap, "expectation failed: %s", s->text);
    } else {
        snprintf(out, cap, "trap T%04u (%s): %s", s->code, name, s->text);
    }
}

#endif

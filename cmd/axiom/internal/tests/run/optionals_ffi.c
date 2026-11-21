// C side for cmd/axiom/internal/tests/run/optionals_ffi.ax.
#include <stddef.h>
#include <stdint.h>

#include "optionals_ffi_generated.h"

static int32_t g_val = 42;

const int32_t *c_maybe_ptr(int32_t want) { return want ? &g_val : NULL; }

int32_t c_read_or(const int32_t *p, int32_t fallback) { return p ? *p : fallback; }

void c_inc_if_nonnull(int32_t *p) {
    if (p) (*p)++;
}

int32_t c_call_cb(int32_t (*cb)(int32_t), int32_t x) { return cb ? cb(x) : -1; }

int32_t c_call_exported(void) {
    int32_t x = 10;
    return ax_opt_sum(&x) + ax_opt_sum(NULL);
}

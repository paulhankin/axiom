// C side of cmd/axiom/internal/tests/run/c_link_wasm64.ax. Freestanding (no libc): it is
// compiled for wasm64 and linked into the same module.
#include <stddef.h>
#include <stdint.h>

uint64_t c_sum(const uint32_t *p, size_t n) {
    uint64_t s = 0;
    for (size_t i = 0; i < n; i++) s += p[i];
    return s;
}

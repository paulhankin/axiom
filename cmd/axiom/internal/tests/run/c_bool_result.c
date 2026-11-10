// C side of cmd/axiom/internal/tests/run/c_bool_result.ax. Freestanding (no libc): it is
// also compiled for wasm32 and linked into the same module.
#include <stdbool.h>
#include <stdint.h>

// (a C compiler sets only the low byte of the result register for these)
bool c_lt5(int x) { return x < 5; }
bool c_is_odd(uint64_t x) { return x & 1; }

// Leaves bits above the low byte set in the result register.
int64_t c_times256(int64_t x) { return x * 256 + 256; }

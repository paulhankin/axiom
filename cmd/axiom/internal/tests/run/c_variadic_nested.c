// C side of cmd/axiom/internal/tests/run/c_variadic_nested.ax. Freestanding (no libc): it is
// also compiled for wasm32 and linked into the same module.
#include <stdarg.h>

// The arguments as the decimal digits of a number, two to each.
int c_digits(int n, ...) {
    va_list ap;
    va_start(ap, n);
    int s = 0;
    for (int i = 0; i < n; i++) s = s * 100 + va_arg(ap, int);
    va_end(ap);
    return s;
}

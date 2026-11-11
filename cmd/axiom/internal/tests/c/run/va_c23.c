/* flags: --c-std=c23 */
/* C23 variable arguments: a function whose parameters are only '...',
   va_start with one argument (and with a second, which is not
   evaluated), va_arg of a _BitInt (not promoted) and of nullptr_t. */
#include <stdarg.h>
#include <stddef.h>
#include "host.h"

static int calls = 0;

static int bump(void) { return ++calls; }

static long long all(...) {
    va_list ap;
    long long s = 0;
    int k;
    va_start(ap);
    while ((k = va_arg(ap, int)) != 0) s = s * 10 + k;
    va_end(ap);
    return s;
}

static long long second(int n, ...) {
    va_list ap;
    long long r;
    va_start(ap, bump());
    r = n + va_arg(ap, int);
    va_end(ap);
    return r;
}

static long long bits(int n, ...) {
    va_list ap;
    long long s = 0;
    _BitInt(7) a;
    unsigned _BitInt(40) b;
    nullptr_t z;
    va_start(ap, n);
    a = va_arg(ap, _BitInt(7));
    b = va_arg(ap, unsigned _BitInt(40));
    z = va_arg(ap, nullptr_t);
    s = a * 1000000000000LL + (long long)b + (z == nullptr);
    va_end(ap);
    return s;
}

int main(void) {
    _BitInt(7) a = -3;
    unsigned _BitInt(40) b = 1099511627775uwb;
    print_i64(all(1, 2, 3, 0));
    print_i64(all(0));
    print_i64(second(40, 2));
    print_i64(calls);
    print_i64(bits(0, a, b, nullptr));
    return 0;
}

/* expect: 123 */
/* expect: 0 */
/* expect: 42 */
/* expect: 0 */
/* expect: -1900488372224 */

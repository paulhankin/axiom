/* flags: --c-std=gnu17 */
/* x86-64-only */
/* native-only */
/* The x87 format's encodings that the processor treats as invalid (an
   exponent that is not 0 without the integer bit: a pseudo-infinity, a
   pseudo-NaN, an unnormal) are NaNs, not signaling ones, for the GNU
   dialect's classification builtins as for <math.h>'s macros. */
#include "print.h"
#include <math.h>
#include <string.h>

static long double make(unsigned long long lo, unsigned short se) {
    unsigned char b[sizeof(long double)] = { 0 };
    long double x;
    memcpy(b, &lo, 8);
    memcpy(b + 8, &se, 2);
    memcpy(&x, b, sizeof x);
    return x;
}

static long code(long double x) {
    return __builtin_isnan(x) + __builtin_isinf(x) * 2 + __builtin_isfinite(x) * 4 + __builtin_isnormal(x) * 8 +
           __builtin_issignaling(x) * 16 + __builtin_fpclassify(0, 1, 2, 3, 4, x) * 32 + isnan(x) * 256 +
           isinf(x) * 512 + __builtin_isfpclass(x, 0x3) * 1024;
}

int main(void) {
    print_i64(code(make(0, 0x7fff)));                     /* a pseudo-infinity */
    print_i64(code(make(0x4000000000000000ULL, 0x7fff))); /* a pseudo-NaN */
    print_i64(code(make(0x1234ULL, 0x3fff)));             /* an unnormal */
    print_i64(code(make(0x8000000000000000ULL, 0x7fff))); /* an infinity */
    print_i64(code(make(0xC000000000000000ULL, 0x3fff))); /* 1.5 */
    return 0;
}
/* expect: 1281 */
/* expect: 1281 */
/* expect: 1281 */
/* expect: 546 */
/* expect: 76 */

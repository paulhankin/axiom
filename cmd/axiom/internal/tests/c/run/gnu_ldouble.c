/* flags: --c-std=gnu11 */
/* The GNU dialects' long double builtins (the l forms) in the format of the
   target's C ABI: the x87's on x86-64, binary128 on AArch64 and
   WebAssembly (gnu_ldouble_64.c and gnu_ldouble_128.c: the others). fabs,
   copysign, the classifiers and the comparisons work on the value itself;
   the others are calls of the library's functions where long double is
   wider than double, which this unit defines (well enough for the values
   used here). */
#include <float.h>
#include "host.h"

long double truncl(long double x) { return (long double)(long long)x; }
long double floorl(long double x) { long double t = truncl(x); return t > x ? t - 1 : t; }
long double ceill(long double x) { long double t = truncl(x); return t < x ? t + 1 : t; }
long double rintl(long double x) {
    long double f = floorl(x), d = x - f;
    if (d > 0.5L || (d == 0.5L && ((long long)f & 1))) return f + 1;
    return f;
}
long double nearbyintl(long double x) { return rintl(x); }
long double fmal(long double x, long double y, long double z) { return x * y + z; }
long double sqrtl(long double x) {
    long double r = x;
    for (int i = 0; i < 200; i++) r = (r + x / r) / 2;
    return r;
}

static volatile long double neg = -2.5L, pos = 3.0L, nzero = -0.0L, tiny = LDBL_MIN, big = LDBL_MAX;

int main(void) {
    long double inf = __builtin_infl(), hv = __builtin_huge_vall(), nan = __builtin_nanl("");
    volatile long double vinf = inf, vnan = nan;
    /* fabs, copysign */
    print_i64(__builtin_fabsl(neg) == 2.5L);
    print_i64(__builtin_signbit(__builtin_fabsl(nzero)));
    print_i64(__builtin_copysignl(pos, neg) == -3.0L);
    print_i64(__builtin_copysignl(neg, pos) == 2.5L);
    print_i64(__builtin_signbit(__builtin_copysignl(0.0L, nzero)));
    print_i64(__builtin_isnan(__builtin_fabsl(-vnan)));
    /* the constants */
    print_i64(__builtin_isinf(inf) && inf > big && hv == inf);
    print_i64(__builtin_isnan(nan) && nan != nan && !__builtin_signbit(nan));
    print_i64(__builtin_isinf_sign(-vinf));
    print_i64(__builtin_isinf_sign(vinf));
    print_i64(__builtin_isinf_sign(big));
    /* the classifiers */
    print_i64(__builtin_isnan(vnan) + __builtin_isnan(vinf) + __builtin_isnan(big));
    print_i64(__builtin_isfinite(big) + __builtin_isfinite(vinf) + __builtin_isfinite(vnan));
    print_i64(__builtin_isnormal(tiny) + __builtin_isnormal(tiny / 2) + __builtin_isnormal(nzero));
    print_i64(__builtin_signbit(neg) + __builtin_signbit(pos) + __builtin_signbit(nzero));
    print_i64(__builtin_fpclassify(0, 1, 2, 3, 4, vnan));
    print_i64(__builtin_fpclassify(0, 1, 2, 3, 4, -vinf));
    print_i64(__builtin_fpclassify(0, 1, 2, 3, 4, neg));
    print_i64(__builtin_fpclassify(0, 1, 2, 3, 4, tiny / 4));
    print_i64(__builtin_fpclassify(0, 1, 2, 3, 4, nzero));
    /* the comparisons */
    print_i64(__builtin_isgreater(pos, neg) + __builtin_isgreater(vnan, neg));
    print_i64(__builtin_isgreaterequal(pos, pos) + __builtin_isless(neg, pos) + __builtin_islessequal(vnan, pos));
    print_i64(__builtin_islessgreater(neg, pos) + __builtin_islessgreater(pos, pos) + __builtin_islessgreater(vnan, pos));
    print_i64(__builtin_isunordered(vnan, pos) + __builtin_isunordered(pos, neg));
    /* the library's */
    print_i64(__builtin_floorl(neg) == -3 && __builtin_ceill(neg) == -2 && __builtin_truncl(neg) == -2);
    print_i64(__builtin_rintl(2.5L) == 2 && __builtin_nearbyintl(-neg + 1) == 4);
    print_i64(__builtin_fmal(2, pos, 4) == 10 && __builtin_sqrtl(2.25L) == 1.5L);
    print_i64(sizeof(__builtin_fabsl(neg)) == sizeof(long double));
    return 0;
}
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: -1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 0 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: 4 */
/* expect: 1 */
/* expect: 2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */

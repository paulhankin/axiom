/* <math.h>'s exact long double functions in the target's long double (the
   x87's 80-bit format on x86-64 Linux, binary128 on AArch64 Linux and
   WebAssembly; double elsewhere): exact on the format's bits, and sqrtl
   correctly rounded. */
/* flags: --c-std=c11 */
#include "print.h"
#include <float.h>
#include <math.h>

/* sqrt(2)'s correctly rounded significand (as an integer) */
static int sqrt2_ok(long double y)
{
    long double m = y * ldexpl(1.0L, LDBL_MANT_DIG - 1);
#if LDBL_MANT_DIG == 64
    return m == 0xb504f333f9de6484p0L;
#elif LDBL_MANT_DIG == 113
    return m == 0x16a09e667f3bcc908b2fb1366ea95p0L;
#else
    return m == 0x16a09e667f3bcdp0L;
#endif
}

int main(void)
{
    volatile long double max = LDBL_MAX, tmin = LDBL_TRUE_MIN, mn = LDBL_MIN, one = 1.0L;
    long double half = ldexpl(1.0L, LDBL_MANT_DIG - 2), ip;
    int e, q;
    print_bool(fabsl(-max) == max && copysignl(max, -one) == -max);
/* expect: true */
    print_bool(frexpl(tmin, &e) == 0.5L && e == LDBL_MIN_EXP - LDBL_MANT_DIG + 1);
/* expect: true */
    print_bool(frexpl(max, &e) < 1.0L && e == LDBL_MAX_EXP);
/* expect: true */
    print_bool(ldexpl(1.0L, LDBL_MIN_EXP - LDBL_MANT_DIG) == tmin && ldexpl(tmin, -1) == 0.0L);
/* expect: true */
    print_bool(ilogbl(tmin) == LDBL_MIN_EXP - LDBL_MANT_DIG && logbl(max) == LDBL_MAX_EXP - 1);
/* expect: true */
    print_bool(truncl(half + 0.5L) == half && roundl(half + 0.5L) == half + 1 && rintl(half + 0.5L) == half);
/* expect: true */
    print_bool(rintl(half + 1.5L) == half + 2 && floorl(-half - 0.5L) == -half - 1 && ceill(-0.5L) == 0.0L);
/* expect: true */
    print_bool(signbit(ceill(-0.5L)) && truncl(max) == max && modfl(-2.75L, &ip) == -0.75L && ip == -2.0L);
/* expect: true */
    print_bool(fmodl(ldexpl(1.0L, 1000), 3.0L) == 1.0L && fmodl(ldexpl(1.0L, LDBL_MAX_EXP > 2000 ? 16000 : 1000), 10.0L) == 6.0L);
/* expect: true */
    print_bool(fmodl(-max, 0.75L) == -0.5L || fmodl(-max, 0.75L) == 0.0L || fmodl(-max, 0.75L) == -0.25L);
/* expect: true */
    print_bool(remquol(29.0L, 3.0L, &q) == -1.0L && q == 2 && remainderl(5.0L, 2.0L) == 1.0L);
/* expect: true */
    print_bool(nextafterl(one, 2.0L) - one == LDBL_EPSILON && nextafterl(0.0L, one) == tmin);
/* expect: true */
    print_bool(nextafterl(mn, 0.0L) == mn - tmin && nextafterl(-tmin, 0.0L) == 0.0L);
/* expect: true */
    print_bool(nextafterl(max, INFINITY) == INFINITY && nexttowardl(one, 0.0L) == one - LDBL_EPSILON / 2);
/* expect: true */
    print_bool(fmaxl(-0.0L, 0.0L) == 0.0L && !signbit(fmaxl(-0.0L, 0.0L)) && fminl(NAN, -one) == -one);
/* expect: true */
    print_bool(sqrtl(9.0L) == 3.0L && sqrtl(max * 0 + 4.0L) == 2.0L && sqrt2_ok(sqrtl(2.0L)));
/* expect: true */
    print_bool(sqrtl(ldexpl(1.0L, LDBL_MAX_EXP > 2000 ? -1090 : -1074)) == ldexpl(1.0L, LDBL_MAX_EXP > 2000 ? -545 : -537) && sqrtl(ldexpl(1.0L, -16444 + 1050 * (LDBL_MAX_EXP < 2000))) == ldexpl(1.0L, -8222 + 525 * (LDBL_MAX_EXP < 2000)));
/* expect: true */
    print_i64(lrintl(-2.5L));
/* expect: -2 */
    print_i64(llroundl(-2.5L));
/* expect: -3 */
    return 0;
}

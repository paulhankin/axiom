/* long double's + - * / where it is wider than double (the x87's format on
   x86-64 Linux, binary128 on AArch64 Linux and WebAssembly; double
   elsewhere), at the roundings the software's fast paths decide: ties to
   even, carries, cancellation, products and quotients just above and below
   halfway, and the operands and results beyond them (subnormal, overflow)
   that the general code takes. p is LDBL_MANT_DIG; every expected value is
   exact. */
/* flags: --c-std=c11 */
#include <float.h>
#include <math.h>
#include <stdio.h>

static void print_bool(int b) { puts(b ? "true" : "false"); }

int main(void)
{
    volatile long double one = 1.0L, two = 2.0L, three = 3.0L;
    long double u = ldexpl(1.0L, 1 - LDBL_MANT_DIG); /* ulp(1) */
    long double h = u / 2;                           /* half of it */
    volatile long double vu = u, vh = h;
    print_bool(one + vh == one && (one + vu) + vh == one + 2 * u);
/* expect: true */
    print_bool(one + (vh + vu * vh) == one + u && one - vh / 2 == one && one - vh == one - h);
/* expect: true */
    print_bool((two - vu) + vu == two && (one + vu) - one == u && (one + vu) - (one - vh) == u + h);
/* expect: true */
    print_bool((one + vu) * (one + vu) == one + 2 * u && (one + vu) * (one - vh) == one);
/* expect: true */
    print_bool((one - vh) * (one - vh) == one - u && three * (one / three) == one);
/* expect: true */
    print_bool(one / (one - vh) == one + u && (one + vu) / (one + vu) == one && two / (one + vu) == two - 2 * u);
/* expect: true */
    volatile long double mx = LDBL_MAX, mn = LDBL_MIN, tm = LDBL_TRUE_MIN;
    print_bool(isinf(mx + mx) && mx * one == mx && mx + (mx * vh * vh) == mx && isinf(mx * (one + vu)));
/* expect: true */
    print_bool(mn / two == ldexpl(LDBL_MIN, -1) && mn * vh == ldexpl(LDBL_MIN, -LDBL_MANT_DIG) &&
               (mn + tm) - mn == tm && tm / two == 0.0L);
/* expect: true */
    print_bool(-(one + vu) * (one + vu) == -(one + 2 * u) && (-one) / (one - vh) == -(one + u) &&
               (one + vu) - (one + vu) == 0.0L && !signbit((one + vu) - (one + vu)));
/* expect: true */
    volatile long double big = ldexpl(1.0L, LDBL_MANT_DIG + 1); /* ulp 4 */
    print_bool(big + one == big && (big + one) - big == 0.0L && (big + three) - big == 4.0L);
/* expect: true */
    return 0;
}

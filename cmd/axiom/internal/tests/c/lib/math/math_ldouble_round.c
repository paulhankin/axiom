/* The wide long double functions where a fast path's rounding test must
   hand over to the exact path (std/c/rt/math_ldx.ax for the x87 format,
   math_ld.ax): exact results, results exactly halfway between two values of
   the format (rounded to even), the far ranges of the inverse hyperbolic
   functions, lgamma near its zeros; in every format the result is the
   correctly rounded value, so each check compares bits. */
/* flags: --c-std=c11 */
#include "print.h"
#include <float.h>
#include <math.h>

static int same(long double a, long double b)
{
    return a == b && signbit(a) == signbit(b);
}

int main(void)
{
    volatile long double two = 2.0L, three = 3.0L;
    /* a square of p/2 + 1 bits: halfway, to even (x = 2^h + 1, x^2 = 2^2h + 2^(h+1) + 1) */
    long double h = (long double)((LDBL_MANT_DIG + 1) / 2);
    long double x = ldexpl(1.0L, (int)h) + 1.0L;
    long double want = ldexpl(1.0L, 2 * (int)h) + ldexpl(1.0L, (int)h + 1);
    print_bool(same(powl(x, two), want) && same(x * x, want));
/* expect: true */
    print_bool(same(hypotl(3.0L, 4.0L), 5.0L) && same(cbrtl(-27.0L), -3.0L) && same(exp2l(-three), 0.125L));
/* expect: true */
    print_bool(same(log2l(ldexpl(1.0L, 300)), 300.0L) && same(log10l(1e20L), 20.0L) && same(powl(10.0L, 20.0L), 1e20L));
/* expect: true */
    print_bool(same(tgammal(21.0L), 2432902008176640000.0L) && same(tgammal(three), two) && same(lgammal(two), 0.0L));
/* expect: true */
    print_bool(same(sqrtl(powl(two, 2.0L * (LDBL_MANT_DIG - 1))), ldexpl(1.0L, LDBL_MANT_DIG - 1)));
/* expect: true */
    /* asinh and acosh of arguments whose square overflows a double:
       log(2x) */
    {
        long double big = ldexpl(1.0L, 783), l2 = logl(2.0L);
        print_bool(fabsl(asinhl(big) - 784.0L * l2) <= 784.0L * l2 * LDBL_EPSILON);
/* expect: true */
        print_bool(fabsl(acoshl(big) - 784.0L * l2) <= 784.0L * l2 * LDBL_EPSILON);
/* expect: true */
        print_bool(same(asinhl(-big), -asinhl(big)));
/* expect: true */
    }
    /* lgamma near its zeros: the result is tiny but has the format's
       relative precision (lgamma(-2.4570247...) = 0 at the zero) */
    {
        long double z0 = -2.45702473822080062303945414765117954L, l = lgammal(z0);
        print_bool(fabsl(l) < 8 * LDBL_EPSILON && lgammal(nextafterl(z0, 0.0L)) != l);
/* expect: true */
    }
    /* the special cases stay the exact path's */
    print_bool(same(expm1l(-60000.0L), -1.0L) && same(tanhl(-50.0L), -1.0L) && same(erfl(200.0L), 1.0L) &&
               same(erfcl(-200.0L), 2.0L) && erfcl(200.0L) == 0.0L);
/* expect: true */
    print_bool(same(acosl(1.0L), 0.0L) && same(asinl(-1.0L), -2.0L * atanl(1.0L)) && same(atanhl(-0.0L), -0.0L));
/* expect: true */
    return 0;
}

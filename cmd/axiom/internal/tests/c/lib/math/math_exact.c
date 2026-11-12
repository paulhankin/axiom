/* <math.h>: the functions with exact results (7.12.6, 7.12.9-7.12.13). */
/* flags: --c-std=c11 */
#include "print.h"
#include <errno.h>
#include <limits.h>
#include <math.h>

static void p(double x)
{
    if (x == 0 && signbit(x))
        print_str("-0", 2);
    else
        print_f64(x);
}

int main(void)
{
    int e, q;
    double ip;
    volatile double z = 0.0, inf = INFINITY;
    p(ceil(-0.5));
/* expect: -0 */
    p(floor(-0.5));
/* expect: -1 */
    p(trunc(-2.7));
/* expect: -2 */
    p(round(2.5));
/* expect: 3 */
    p(round(-2.5));
/* expect: -3 */
    p(round(0.49999999999999994));
/* expect: 0 */
    p(rint(2.5));
/* expect: 2 */
    p(rint(-3.5));
/* expect: -4 */
    p(rint(-0.25));
/* expect: -0 */
    p(nearbyint(2.5));
/* expect: 2 */
    p(rint(4503599627370497.0));
/* expect: 4503599627370497 */
    print_i64(lrint(-2.5));
/* expect: -2 */
    print_i64(lround(-2.5));
/* expect: -3 */
    print_i64(llround(1e18 + 0.5));
/* expect: 1000000000000000000 */
    print_i64(llrint(-9223372036854775808.0) == LLONG_MIN);
/* expect: 1 */
    errno = 0;
    print_i64(llrint(1e19) == LLONG_MIN);
/* expect: 1 */
    print_i64(errno);
/* expect: 33 */
    /* remainders */
    p(fmod(5.5, 2.0));
/* expect: 1.5 */
    p(fmod(-5.5, 2.0));
/* expect: -1.5 */
    p(fmod(1e300, 3.0));
/* expect: 0 */
    p(fmod(0x1p1023, 0x1p-1074));
/* expect: 0 */
    p(fmod(-z, 1.0));
/* expect: -0 */
    p(fmod(3.0, inf));
/* expect: 3 */
    errno = 0;
    p(fmod(inf, 1.0));
/* expect: NaN */
    print_i64(errno);
/* expect: 33 */
    p(remainder(5.0, 2.0));
/* expect: 1 */
    p(remainder(7.0, 2.0));
/* expect: -1 */
    p(remainder(-1.5, 1.0));
/* expect: 0.5 */
    p(remainder(1.0, 3.0));
/* expect: 1 */
    p(remainder(2.0, 3.0));
/* expect: -1 */
    p(remainder(1e308, 0.1));
/* expect: -0.03906711615670008 */
    p(remquo(29.0, 3.0, &q));
/* expect: -1 */
    print_i64(q);
/* expect: 2 */
    p(remquo(-29.0, 3.0, &q));
/* expect: 1 */
    print_i64(q);
/* expect: -2 */
    /* exponents */
    p(frexp(48.0, &e));
/* expect: 0.75 */
    print_i64(e);
/* expect: 6 */
    p(frexp(0x1p-1074, &e));
/* expect: 0.5 */
    print_i64(e);
/* expect: -1073 */
    p(ldexp(1.0, -1074));
/* expect: 5e-324 */
    p(ldexp(1.5, -1074));
/* expect: 1e-323 */
    errno = 0;
    p(ldexp(1.0, 1024));
/* expect: inf */
    print_i64(errno);
/* expect: 34 */
    p(scalbn(0x1p-1074, 2097));
/* expect: 8.98846567431158e+307 */
    p(scalbln(3.0, -1L));
/* expect: 1.5 */
    p(ldexp(-2.5, 0));
/* expect: -2.5 */
    print_i64(ilogb(0x1p-1074));
/* expect: -1074 */
    print_i64(ilogb(z) == FP_ILOGB0);
/* expect: 1 */
    print_i64(ilogb(NAN) == FP_ILOGBNAN);
/* expect: 1 */
    p(logb(-1024.5));
/* expect: 10 */
    p(logb(-z));
/* expect: -inf */
    p(modf(-3.25, &ip));
/* expect: -0.25 */
    p(ip);
/* expect: -3 */
    p(modf(-inf, &ip));
/* expect: -0 */
    /* neighbours, maxima */
    p(nextafter(1.0, 2.0));
/* expect: 1.0000000000000002 */
    p(nextafter(z, -1.0));
/* expect: -5e-324 */
    p(nexttoward(1.0, 0.0L));
/* expect: 0.9999999999999999 */
    p(copysign(3.0, -z));
/* expect: -3 */
    p(fdim(5.0, 3.0));
/* expect: 2 */
    p(fdim(3.0, 5.0));
/* expect: 0 */
    p(fmax(NAN, 2.0));
/* expect: 2 */
    p(fmin(-z, z));
/* expect: -0 */
    p(fmax(-z, z));
/* expect: 0 */
    p(fabs(-inf));
/* expect: inf */
    p(nan("") != nan(""));
/* expect: 1 */
    /* fused multiply-add: one rounding */
    p(fma(0x1p-30 + 1.0, 0x1p-30 + 1.0, -1.0));
/* expect: 1.8626451500983188e-9 */
    p(fma(1e308, 10.0, -inf));
/* expect: -inf */
    p(fma(0x1.fffffffffffffp-1, 0x1p-1022, 0x1p-1074));
/* expect: 2.2250738585072014e-308 */
    return 0;
}

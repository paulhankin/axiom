/* <math.h>: the special values of Annex F (F.10) and the errors of 7.12.1
   (errno: math_errhandling has MATH_ERRNO on every target). */
/* flags: --c-std=c11 */
#include "print.h"
#include <errno.h>
#include <math.h>

static void p(double x)
{
    if (x == 0 && signbit(x))
        print_str("-0", 2);
    else
        print_f64(x);
}

/* the value, then errno (reset) */
static void pe(double x)
{
    p(x);
    print_i64(errno);
    errno = 0;
}

int main(void)
{
    volatile double z = 0.0, inf = INFINITY, nan = NAN, one = 1.0;
    print_bool(math_errhandling & MATH_ERRNO);
/* expect: true */
    /* exponentials and logarithms */
    p(exp(-z));
/* expect: 1 */
    p(exp(-inf));
/* expect: 0 */
    p(exp(inf));
/* expect: inf */
    pe(exp(1000.0));
/* expect: inf */
/* expect: 34 */
    pe(exp(-1000.0));
/* expect: 0 */
/* expect: 34 */
    p(exp2(10.0));
/* expect: 1024 */
    p(exp2(-1074.0));
/* expect: 5e-324 */
    p(expm1(-inf));
/* expect: -1 */
    p(1.0 / expm1(-z));
/* expect: -inf */
    pe(log(z));
/* expect: -inf */
/* expect: 34 */
    pe(log(-one));
/* expect: NaN */
/* expect: 33 */
    p(log(one));
/* expect: 0 */
    p(log(inf));
/* expect: inf */
    p(log2(0x1p-1074));
/* expect: -1074 */
    p(log10(1e22));
/* expect: 22 */
    pe(log1p(-one));
/* expect: -inf */
/* expect: 34 */
    pe(log1p(-2.0));
/* expect: NaN */
/* expect: 33 */
    p(1.0 / log1p(-z));
/* expect: -inf */
    /* powers */
    p(pow(nan, z));
/* expect: 1 */
    p(pow(one, nan));
/* expect: 1 */
    p(pow(-one, inf));
/* expect: 1 */
    pe(pow(-z, -3.0));
/* expect: -inf */
/* expect: 34 */
    pe(pow(z, -2.5));
/* expect: inf */
/* expect: 34 */
    p(pow(-z, 3.0));
/* expect: -0 */
    p(pow(-inf, 3.0));
/* expect: -inf */
    p(pow(-inf, -3.0));
/* expect: -0 */
    p(pow(-inf, 2.0));
/* expect: inf */
    p(pow(0.5, -inf));
/* expect: inf */
    p(pow(2.0, -inf));
/* expect: 0 */
    pe(pow(-2.0, 0.5));
/* expect: NaN */
/* expect: 33 */
    p(pow(-2.0, 3.0));
/* expect: -8 */
    p(pow(9.0, 0.5));
/* expect: 3 */
    pe(pow(10.0, 400.0));
/* expect: inf */
/* expect: 34 */
    pe(sqrt(-one));
/* expect: NaN */
/* expect: 33 */
    p(sqrt(-z));
/* expect: -0 */
    p(cbrt(-27.0));
/* expect: -3 */
    p(hypot(inf, nan));
/* expect: inf */
    p(hypot(3.0, 4.0));
/* expect: 5 */
    /* trigonometric */
    p(sin(-z));
/* expect: -0 */
    pe(sin(inf));
/* expect: NaN */
/* expect: 33 */
    pe(cos(-inf));
/* expect: NaN */
/* expect: 33 */
    p(tan(-z));
/* expect: -0 */
    p(atan(inf));
/* expect: 1.5707963267948966 */
    p(atan2(z, -z));
/* expect: 3.141592653589793 */
    p(atan2(-z, -z));
/* expect: -3.141592653589793 */
    p(atan2(-z, z));
/* expect: -0 */
    p(atan2(inf, -inf));
/* expect: 2.356194490192345 */
    p(atan2(-one, z));
/* expect: -1.5707963267948966 */
    pe(asin(2.0));
/* expect: NaN */
/* expect: 33 */
    p(acos(one));
/* expect: 0 */
    p(acos(-one));
/* expect: 3.141592653589793 */
    /* hyperbolic */
    p(sinh(-inf));
/* expect: -inf */
    p(cosh(-inf));
/* expect: inf */
    p(tanh(-inf));
/* expect: -1 */
    pe(cosh(711.0));
/* expect: inf */
/* expect: 34 */
    p(cosh(710.0));
/* expect: 1.1169973830808555e+308 */
    pe(atanh(one));
/* expect: inf */
/* expect: 34 */
    pe(acosh(0.5));
/* expect: NaN */
/* expect: 33 */
    p(asinh(-inf));
/* expect: -inf */
    /* special functions */
    p(erf(-inf));
/* expect: -1 */
    p(erfc(-inf));
/* expect: 2 */
    p(erfc(inf));
/* expect: 0 */
    pe(tgamma(-z));
/* expect: -inf */
/* expect: 34 */
    pe(tgamma(-2.0));
/* expect: NaN */
/* expect: 33 */
    pe(tgamma(-inf));
/* expect: NaN */
/* expect: 33 */
    p(tgamma(5.0));
/* expect: 24 */
    p(tgamma(0.5));
/* expect: 1.772453850905516 */
    pe(lgamma(-3.0));
/* expect: inf */
/* expect: 34 */
    p(lgamma(one));
/* expect: 0 */
    p(lgamma(2.0));
/* expect: 0 */
    p(lgamma(-inf));
/* expect: inf */
    return 0;
}

/* <fenv.h> on the targets with a floating-point environment (x86-64's
   MXCSR, AArch64's FPCR and FPSR): the flags raised by arithmetic and by
   <math.h>'s functions (MATH_ERREXCEPT), the rounding modes, the
   environment saved and restored. */
/* flags: --c-std=c11 */
/* native-only */
#include "print.h"
#include <fenv.h>
#include <math.h>

/* (operands the optimizer cannot see: it folds constants as if rounding to
   nearest with no flags, docs/c.md) */
static volatile double z = 0.0, one = 1.0, big = 1e300, tiny = 1e-300, three = 3.0;
static volatile double c2_5 = 2.5, c2_1 = 2.1, c2_9 = 2.9, two = 2.0, c800 = 800.0, cm1 = -1.0;

static void flags(void)
{
    print_i64(fetestexcept(FE_ALL_EXCEPT));
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    volatile double r;
    double v[15];
    int i;
    fenv_t env;
    fexcept_t f;
    print_bool(math_errhandling & MATH_ERREXCEPT);
/* expect: true */
    feclearexcept(FE_ALL_EXCEPT);
    flags();
/* expect: 0 */
    r = one / z;
    flags();
/* expect: 4 */
    r = big * big;
    flags();
/* expect: 40 */
    r = tiny * tiny;
    flags();
/* expect: 48 */
    r = z / z;
    flags();
/* expect: 1 */
    r = one / three;
    flags();
/* expect: 32 */
    r = sqrt(-one);
    flags();
/* expect: 1 */
    r = log(z);
    flags();
/* expect: 4 */
    r = exp(c800);
    flags();
/* expect: 40 */
    r = exp(-c800);
    flags();
/* expect: 48 */
    r = pow(z, cm1);
    flags();
/* expect: 4 */
    r = sin(INFINITY);
    flags();
/* expect: 1 */
    r = exp(z);
    flags();
/* expect: 0 */
    r = fma(two, three, one);
    flags();
/* expect: 0 */
    r = nearbyint(c2_5);
    flags();
/* expect: 0 */
    r = rint(c2_5);
    flags();
/* expect: 32 */
    print_i64(feraiseexcept(FE_OVERFLOW | FE_INEXACT));
/* expect: 0 */
    flags();
/* expect: 40 */
    /* rounding modes, and the functions that follow them (the results
       printed once rounding is to nearest again) */
    print_i64(fegetround() == FE_TONEAREST);
/* expect: 1 */
    print_i64(fesetround(FE_UPWARD));
/* expect: 0 */
    print_i64(fegetround() == FE_UPWARD);
/* expect: 1 */
    v[0] = one / three;
    v[1] = rint(c2_1);
    v[2] = sqrt(two);
    v[3] = fma(one, one, 0x1p-80);
    v[4] = (double)lrint(-c2_5);
    fesetround(FE_DOWNWARD);
    v[5] = one / three;
    v[6] = rint(-c2_1);
    v[7] = fma(one, one, -0x1p-80);
    v[8] = sqrt(two);
    v[9] = z - z;
    fesetround(FE_TOWARDZERO);
    v[10] = fegetround() == FE_TOWARDZERO;
    v[11] = -one / three;
    v[12] = big * big;
    v[13] = rint(-c2_9);
    v[14] = fesetround(12345);
    fesetround(FE_TONEAREST);
    for (i = 0; i < 15; i++) {
        if (v[i] == 0 && signbit(v[i]))
            print_str("-0", 2);
        else
            print_f64(v[i]);
    }
/* expect: 0.33333333333333337 */
/* expect: 3 */
/* expect: 1.4142135623730951 */
/* expect: 1.0000000000000002 */
/* expect: -2 */
/* expect: 0.3333333333333333 */
/* expect: -3 */
/* expect: 0.9999999999999999 */
/* expect: 1.414213562373095 */
/* expect: -0 */
/* expect: 1 */
/* expect: -0.3333333333333333 */
/* expect: 1.7976931348623157e+308 */
/* expect: -2 */
/* expect: 1 */
    feclearexcept(FE_ALL_EXCEPT);
    /* environments */
    r = one / z;
    fegetexceptflag(&f, FE_ALL_EXCEPT);
    feclearexcept(FE_ALL_EXCEPT);
    fesetexceptflag(&f, FE_DIVBYZERO);
    flags();
/* expect: 4 */
    fesetround(FE_UPWARD);
    r = one / three;
    fegetenv(&env);
    fesetenv(FE_DFL_ENV);
    print_i64(fegetround() == FE_TONEAREST);
/* expect: 1 */
    flags();
/* expect: 0 */
    fesetenv(&env);
    print_i64(fegetround() == FE_UPWARD);
/* expect: 1 */
    flags();
/* expect: 32 */
    print_i64(feholdexcept(&env));
/* expect: 0 */
    r = big * big;
    print_i64(fetestexcept(FE_OVERFLOW));
/* expect: 8 */
    fesetround(FE_TONEAREST);
    feupdateenv(&env);
    print_i64(fegetround() == FE_UPWARD);
/* expect: 1 */
    flags();
/* expect: 40 */
    fesetround(FE_TONEAREST);
    (void)r;
    return 0;
}

/* _Decimal32 and _Decimal64 + and * at the cases their 64-bit fast paths
   take (coefficients and exponents in range) and around them: the quantum
   of exact results, rounding of 17 to 32 digits in each direction, and the
   flags. */
/* flags: --c-std=c23 */
/* native-only */
#include <fenv.h>
#include <stdio.h>
#pragma STDC FENV_ACCESS ON

static void show(_Decimal64 x, int fl)
{
    char b[64];
    sprintf(b, "%Da %d", x, (fl & FE_INEXACT) != 0);
    puts(b);
}

int main(void)
{
    volatile _Decimal64 a = 1.50DD, b = 2.0DD, c = 9999999999999999.DD, d = 1234567890123456e-5DD;
    feclearexcept(FE_ALL_EXCEPT);
    show(a * b, fetestexcept(FE_INEXACT));
/* expect: 3.000 0 */
    show(a + b, fetestexcept(FE_INEXACT));
/* expect: 3.50 0 */
    feclearexcept(FE_ALL_EXCEPT);
    show(c + 1, fetestexcept(FE_INEXACT));
/* expect: 1.000000000000000e+16 0 */
    feclearexcept(FE_ALL_EXCEPT);
    show(c * c, fetestexcept(FE_INEXACT));
/* expect: 9.999999999999998e+31 1 */
    feclearexcept(FE_ALL_EXCEPT);
    show(d * d, fetestexcept(FE_INEXACT));
/* expect: 1.524157875323882e+20 1 */
    fe_dec_setround(FE_DEC_UPWARD);
    feclearexcept(FE_ALL_EXCEPT);
    show(d * d, fetestexcept(FE_INEXACT));
/* expect: 1.524157875323882e+20 1 */
    fe_dec_setround(FE_DEC_DOWNWARD);
    show(-(d * d), 0);
/* expect: -1.524157875323881e+20 0 */
    show(a - a, 0);
/* expect: -0.00 0 */
    fe_dec_setround(FE_DEC_TONEAREST);
    show(c + 0.5DD, 0);
/* expect: 1.000000000000000e+16 0 */
    show(c + 0.4DD, 0);
/* expect: 9999999999999999 0 */
    show(1e3DD * 1e-3DD, 0);
/* expect: 1 0 */
    volatile _Decimal32 e = 9999999.DF, f = 0.5DF;
    _Decimal32 g = e + f;
    show((_Decimal64)g, 0);
/* expect: 1.000000e+07 0 */
    g = e * e;
    show((_Decimal64)g, 0);
/* expect: 9.999998e+13 0 */
    return 0;
}

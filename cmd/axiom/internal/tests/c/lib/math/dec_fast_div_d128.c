/* The decimal fast paths of this library: _Decimal32 and _Decimal64
   division, _Decimal128 + - * / in 128- and 256-bit integers (decimal.ax),
   and the binary path of the elementary _Decimal32 and _Decimal64
   functions (dfp_fast.ax), each against what the general decimal code
   gives: the quantum of exact results, the rounding in every direction,
   the flags. */
/* flags: --c-std=c23 */
/* native-only */
#include <fenv.h>
#include <math.h>
#include <stdio.h>
#pragma STDC FENV_ACCESS ON

static void s64(_Decimal64 x)
{
    char b[80];
    int fl = fetestexcept(FE_INEXACT) != 0;
    sprintf(b, "%Da %d", x, fl);
    puts(b);
    feclearexcept(FE_ALL_EXCEPT);
}

static void s32(_Decimal32 x)
{
    char b[80];
    int fl = fetestexcept(FE_INEXACT) != 0;
    sprintf(b, "%Ha %d", x, fl);
    puts(b);
    feclearexcept(FE_ALL_EXCEPT);
}

static void s128(_Decimal128 x)
{
    char b[120];
    int fl = fetestexcept(FE_INEXACT) != 0;
    sprintf(b, "%DDa %d", x, fl);
    puts(b);
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void)
{
    static const int modes[5] = {FE_DEC_TONEAREST, FE_DEC_DOWNWARD, FE_DEC_UPWARD, FE_DEC_TOWARDZERO,
                                 FE_DEC_TONEARESTFROMZERO};
    volatile _Decimal64 one = 1.00DD, three = 3.0DD, four = 4.DD, big = 9999999999999999.DD;
    volatile _Decimal32 one32 = 1.0DF, seven = 7.DF;
    volatile _Decimal128 g = 1234567890123456789012345678901234.DL, h = 3.000DL, q = 0.25DL, u = 1.5DL;
    volatile _Decimal128 tiny = 1e-30DL, n7 = 7.DL;
    feclearexcept(FE_ALL_EXCEPT);
    s64(one / four);
    s64(one / three);
    s64(big / three);
    s64(12.0DD / four);
    s32(one32 / seven);
    s128(q / u);
    s128(u / q);
    s128(g / h);
    s128(g * g);
    s128(g + tiny);
    s128(g - g);
    s128(u * q);
    s128(g / n7);
    for (int m = 0; m < 5; m++) {
        fe_dec_setround(modes[m]);
        s64(-one / three);
        s32(one32 / seven);
        s128(g * h);
        s128(-g / n7);
        s128(g + 0.5DL);
        s64(expd64(1e-20DD));
        s64(sind64(0.5DD));
        s32(logd32(3.0DF));
        s64(atan2d64(-1.0DD, 3.0DD));
        s64(powd64(1.5DD, 2.5DD));
    }
    fe_dec_setround(FE_DEC_TONEAREST);
    s64(exp2d64(10.0DD));
    s64(cospid64(3610.DD));
    s64(sqrtd64(2.25DD));
    return 0;
}
/* expect: 0.25 0 */
/* expect: 0.3333333333333333 1 */
/* expect: 3333333333333333 0 */
/* expect: 3.0 0 */
/* expect: 0.1428571 1 */
/* expect: 0.1666666666666666666666666666666667 1 */
/* expect: 6 0 */
/* expect: 411522630041152263004115226300411.3 1 */
/* expect: 1.524157875323883675049535156256667e+66 1 */
/* expect: 1234567890123456789012345678901234 1 */
/* expect: 0 0 */
/* expect: 0.375 0 */
/* expect: 176366841446208112716049382700176.3 1 */
/* expect: -0.3333333333333333 1 */
/* expect: 0.1428571 1 */
/* expect: 3703703670370370367037037036703702 0 */
/* expect: -176366841446208112716049382700176.3 1 */
/* expect: 1234567890123456789012345678901234 1 */
/* expect: 1.000000000000000 1 */
/* expect: 0.4794255386042030 1 */
/* expect: 1.098612 1 */
/* expect: -0.3217505543966422 1 */
/* expect: 2.755675960631075 1 */
/* expect: -0.3333333333333334 1 */
/* expect: 0.1428571 1 */
/* expect: 3703703670370370367037037036703702 0 */
/* expect: -176366841446208112716049382700176.3 1 */
/* expect: 1234567890123456789012345678901234 1 */
/* expect: 1.000000000000000 1 */
/* expect: 0.4794255386042030 1 */
/* expect: 1.098612 1 */
/* expect: -0.3217505543966422 1 */
/* expect: 2.755675960631075 1 */
/* expect: -0.3333333333333333 1 */
/* expect: 0.1428572 1 */
/* expect: 3703703670370370367037037036703702 0 */
/* expect: -176366841446208112716049382700176.2 1 */
/* expect: 1234567890123456789012345678901235 1 */
/* expect: 1.000000000000001 1 */
/* expect: 0.4794255386042031 1 */
/* expect: 1.098613 1 */
/* expect: -0.3217505543966421 1 */
/* expect: 2.755675960631076 1 */
/* expect: -0.3333333333333333 1 */
/* expect: 0.1428571 1 */
/* expect: 3703703670370370367037037036703702 0 */
/* expect: -176366841446208112716049382700176.2 1 */
/* expect: 1234567890123456789012345678901234 1 */
/* expect: 1.000000000000000 1 */
/* expect: 0.4794255386042030 1 */
/* expect: 1.098612 1 */
/* expect: -0.3217505543966421 1 */
/* expect: 2.755675960631075 1 */
/* expect: -0.3333333333333333 1 */
/* expect: 0.1428571 1 */
/* expect: 3703703670370370367037037036703702 0 */
/* expect: -176366841446208112716049382700176.3 1 */
/* expect: 1234567890123456789012345678901235 1 */
/* expect: 1.000000000000000 1 */
/* expect: 0.4794255386042030 1 */
/* expect: 1.098612 1 */
/* expect: -0.3217505543966422 1 */
/* expect: 2.755675960631075 1 */
/* expect: 1024 0 */
/* expect: 1 0 */
/* expect: 1.5 0 */

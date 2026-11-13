/* The library's _Decimal64x (C23 Annex H): <float.h>'s DEC64X_ macros,
   HUGE_VAL_D64X, its functions (those of _Decimal128, whose format it
   has: the same values and quanta), the narrowing operations to and from
   it, strtod64x, strfromd64x, wcstod64x, and <tgmath.h> with it (H.13p3:
   an integer argument regarded as _Decimal64x then). */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define P(x) printf("%DDa ", (_Decimal128)(x))
#define T(x) _Generic((x), _Decimal32: "d32", _Decimal64: "d64", _Decimal64x: "d64x", _Decimal128: "d128", \
                      long long: "ll", int: "int", bool: "bool", default: "?")

static const _Decimal64x k_snan = DEC64X_SNAN;

int main(void) {
    printf("%d %d %d\n", DEC64X_MANT_DIG, DEC64X_MIN_EXP, DEC64X_MAX_EXP);
/* expect: 34 -6142 6145 */
    P(DEC64X_MAX); P(DEC64X_EPSILON); P(DEC64X_MIN); P(DEC64X_TRUE_MIN); printf("%s\n", T(DEC64X_MAX));
/* expect: 9.999999999999999999999999999999999e+6144 1e-33 1e-6143 1e-6176 d64x */
    printf("%d %d %d %s\n", isinf(HUGE_VAL_D64X), issignaling(k_snan), issignaling(-DEC64X_SNAN),
           T(HUGE_VAL_D64X));
/* expect: 1 1 1 d64x */
    _Decimal64x two = 2, x = 1.44d64x;
    P(sqrtd64x(x)); P(expd64x(1)); P(logd64x(two)); P(powd64x(two, 10)); P(quantized64x(1.2345d64x, 1.00d64x));
/* expect: 1.2 2.718281828459045235360287471352662 0.6931471805599453094172321214581766 1024 1.23  */
    printf("\n%s %s %s %s ", T(expd64x(1)), T(llquantexpd64x(x)), T(samequantumd64x(x, x)), T(ilogbd64x(x)));
    printf("%lld %d %d\n", llquantexpd64x(x), samequantumd64x(x, 1.00d64x), totalorderd64x(&two, &x));
/* expect: d64x ll bool int -2 1 0 */
    /* the narrowing operations: one rounding */
    P(d32addd64x(1.0000005d64x, 1E-20d64x)); P(d64muld64x(1.1d64x, 1.1d64x)); P(d64xdivd128(1, 3.dl));
    P(d64xsqrtd128(2)); printf("%s %s\n", T(d32fmad64x(1, 2, 3)), T(d64xaddd128(1, 2)));
/* expect: 1.000001 1.21 0.3333333333333333333333333333333333 1.414213562373095048801688724209698 d32 d64x */
    /* conversions of strings */
    char *end, b[64];
    P(strtod64x("1.250e2xyz", &end)); printf("%s ", end);
    errno = 0;
    P(strtod64x("1e9999", 0)); printf("%d ", errno == ERANGE);
    strfromd64x(b, sizeof b, "%.5e", sqrtd64x(two));
    printf("%s ", b);
    wchar_t *wend;
    P(wcstod64x(L" 0.75q", &wend)); printf("%d\n", (int)(*wend == L'q'));
/* expect: 125.0 xyz inf 1 1.41421e+00 0.75 1 */
    /* the decimal rounding direction */
    fe_dec_setround(FE_DEC_UPWARD);
    P(1 / (two + 1)); P(d32divd64x(1, 3)); P(strtod64x("0.33333333333333333333333333333333331", 0));
    fe_dec_setround(FE_DEC_TONEAREST);
    printf("\n");
/* expect: 0.3333333333333333333333333333333334 0.3333334 0.3333333333333333333333333333333334  */
    return 0;
}

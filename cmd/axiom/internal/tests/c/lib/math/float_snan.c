/* <float.h>'s signaling NaNs (C23 5.2.4.2.2, F.2.1): FLT_SNAN, DBL_SNAN,
   LDBL_SNAN and the _FloatN ones are constant expressions that stay
   signaling in objects they initialize (static and automatic, with a unary
   + or - too), and an operation on one raises "invalid" and gives a quiet
   NaN; with Annex G, I is _Imaginary_I. */
/* flags: --c-std=c23 */
/* native-only */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include "print.h"
#include <complex.h>
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <string.h>

static float sf = FLT_SNAN, nf = -FLT_SNAN;
static double sd = DBL_SNAN;
static long double sl = +LDBL_SNAN;
static _Float16 sh = FLT16_SNAN;
static _Float128 sq = FLT128_SNAN;

int main(void)
{
    float lf = FLT_SNAN;
    double ld = -DBL_SNAN;
    unsigned u;
    unsigned short h;
    memcpy(&u, &sf, 4);
    memcpy(&h, &sh, 2);
    print_bool(issignaling(sf) && issignaling(nf) && signbit(nf) && issignaling(sd) && issignaling(sl));
    print_bool(issignaling(sq) && issignaling(lf) && issignaling(ld) && signbit(ld) && u == 0x7fa00000 && h == 0x7d00);
    feclearexcept(FE_ALL_EXCEPT);
    volatile double t = sd + 1;
    print_bool(fetestexcept(FE_INVALID) && isnan(t) && !issignaling(t));
#ifdef __STDC_IEC_559_COMPLEX__
    print_bool(_Generic(I, const float _Imaginary: 1, float _Imaginary: 1, default: 0) && cimag(2.0 * I + 1.0) == 2.0);
#else
    print_bool(0);
#endif
    return 0;
}
// expect: true
// expect: true
// expect: true
// expect: true

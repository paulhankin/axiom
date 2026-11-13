/* complexf128.c - <complex.h>'s _Float128 functions (C23 Annex H.11:
   cexpf128 ...): complex_impl.h's, from the _Float128 functions
   (std/c/rt/math_ld.ax's, in binary128's precision), as the long double
   ones of a binary128 long double are (complexl.c). */
#if __STDC_VERSION__ >= 199901L
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <complex.h>
#include <float.h>
#include <math.h>

#define T _Float128
#define CT _Float128 _Complex
#define C(x, y) __builtin_complex((_Float128)(x), (_Float128)(y))
#define RE(z) __builtin_creal(z)
#define IM(z) __builtin_cimag(z)
#define INF ((_Float128)__builtin_inff())
#define FN(n) n##f128
#define M(n) n##f128
#define K_EXP_BIG 11350.0F128
#define K_SQ_HI 8000
#define K_SQ_LO (-8000)
#define K_SQ_HIV 0x1p8000F128
#define K_SQ_LOV 0x1p-8000F128
#define K_TANH_BIG 45.0F128
#define K_ASIN_BIG 0x1p120F128
#define K_ATANH_BIG 0x1p8000F128
#define K_PIO4 0.785398163397448309615660845819875721F128
#define K_PIO2 1.57079632679489661923132169163975144F128
#define K_3PIO4 2.35619449019234492884698253745962716F128
#define K_PI 3.14159265358979323846264338327950288F128
#define K_LN2 0.693147180559945309417232121458176568F128
#define K_LN2_LO (-7.008139474549585163412662008771625674e-36F128)

/* the real functions in pairs of binary128 values (std/c/rt/math_ld.ax,
   format 0) */
void __axc_ldcx(unsigned long long op, unsigned long long f, _Float128 *r, const _Float128 *x, const _Float128 *y);

static void xk1_q(int op, _Float128 x, _Float128 *r)
{
    __axc_ldcx((unsigned long long)op, 0, r, &x, &x);
}

static void xk2_q(int op, _Float128 y, _Float128 x, _Float128 *r)
{
    __axc_ldcx((unsigned long long)op + 4, 0, r, &y, &x);
}
#define XK1(op, x, r) xk1_q((op), (x), (r))
#define XK2(op, y, x, r) xk2_q((op), (y), (x), (r))
#define PROD_ERR(a, b, p) fmaf128((a), (b), -(p))

#include "complex_impl.h"
#else
typedef int __axc_complexf128_c89;
#endif

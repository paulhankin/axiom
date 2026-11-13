/* complex.c - the double forms of <complex.h>'s functions (complex_impl.h).
   The float forms are cfloat.c's, the long double ones complexl.c's. */
#if __STDC_VERSION__ >= 199901L
#include <complex.h>
#include <float.h>
#include <math.h>

#define T double
#define CT double complex
#define C(x, y) __builtin_complex((double)(x), (double)(y))
#define RE(z) __builtin_creal(z)
#define IM(z) __builtin_cimag(z)
#define INF ((double)__builtin_inff())
#define FN(n) n
#define M(n) n
#define K_EXP_BIG 700.0
#define K_SQ_HI 500
#define K_SQ_LO (-450)
#define K_SQ_HIV 0x1p500
#define K_SQ_LOV 0x1p-450
#define K_TANH_BIG 22.0
#define K_ASIN_BIG 0x1p60
#define K_ATANH_BIG 0x1p500
#define K_PIO4 0.78539816339744830962
#define K_PIO2 1.57079632679489661923
#define K_3PIO4 2.35619449019234492885
#define K_PI 3.14159265358979323846
#define K_LN2 0x1.62e42fefa39efp-1
#define K_LN2_LO 0x1.abc9e3b39803fp-56

/* the real functions in double-double (std/c/rt/math_cx.ax) */
void __axc_cx1(unsigned op, double x, double *r);
void __axc_cx2(unsigned op, double y, double x, double *r);
#define XK1(op, x, r) __axc_cx1((unsigned)(op), (x), (r))
#define XK2(op, y, x, r) __axc_cx2((unsigned)(op), (y), (x), (r))

/* a b - p exactly (p = a b rounded, finite and nonzero): Dekker's product
   of Veltkamp's halves where nothing overflows or underflows, else fma */
static double prod_err(double a, double b, double p)
{
    double fa = fabs(a), fb = fabs(b), fp = fabs(p);
    if (fa < 0x1p995 && fb < 0x1p995 && fa > 0x1p-920 && fb > 0x1p-920 && fp > 0x1p-960 && fp < 0x1p995) {
        double t = 134217729.0 * a, ah = t - (t - a), al = a - ah;
        double u = 134217729.0 * b, bh = u - (u - b), bl = b - bh;
        return (((ah * bh - p) + ah * bl) + al * bh) + al * bl;
    }
    return fma(a, b, -p);
}
#define PROD_ERR(a, b, p) prod_err((a), (b), (p))

/* cpow where |w| |log z| is beyond 2^12 (the pairs' angle would lose
   more than a tenth of an ulp): w log z in triple-double in binary128
   (std/c/rt/math_ld.ax), each part rounded to binary128 and then to
   double (within half an ulp and 2^-60 more) */
unsigned __axc_ldcpow(unsigned long long f, _Float128 *r, const _Float128 *z, const _Float128 *w);

static int xpow_d(double complex z, double complex w, double complex *r)
{
    _Float128 zz[2], ww[2], rr[2];
    zz[0] = __builtin_creal(z);
    zz[1] = __builtin_cimag(z);
    ww[0] = __builtin_creal(w);
    ww[1] = __builtin_cimag(w);
    if (!__axc_ldcpow(0, rr, zz, ww))
        return 0;
    *r = __builtin_complex((double)rr[0], (double)rr[1]);
    return 1;
}
#define XPOW(z, w, r) xpow_d((z), (w), (r))
#define K_POW_WIDE 0x1p12

#include "complex_impl.h"
#else
typedef int __axc_complex_c89;
#endif

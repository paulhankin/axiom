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
#define K_MAX DBL_MAX
#define K_EXP_BIG 700.0
#define K_TINY 0x1p-1020
#define K_UP 0x1p600
#define K_DOWN_HALF 0x1p-300
#define K_TANH_BIG 22.0
#define K_ASIN_BIG 0x1p60
#define K_ATANH_BIG 0x1p500
#define K_PIO4 0.78539816339744830962
#define K_PIO2 1.57079632679489661923
#define K_3PIO4 2.35619449019234492885
#define K_PI 3.14159265358979323846
#define K_LN2 0.69314718055994530942

double __axc_half_log(double s, double e);

/* log|z| (ax >= ay >= 0, ax > 0): log(x^2 + y^2) / 2 from the squares'
   exact sum s + e (fma), in double-double (math_log.ax): nothing cancels
   near |z| = 1 */
static double clog_re(double ax, double ay)
{
    double p, pe, q, qe, sum, bb, se;
    if (ax > 1e300 || ax < 1e-300)
        return log(ax) + 0.5 * log1p((ay / ax) * (ay / ax));
    p = ax * ax;
    pe = fma(ax, ax, -p);
    q = ay * ay;
    qe = fma(ay, ay, -q);
    sum = p + q;
    bb = sum - p;
    se = (p - (sum - bb)) + (q - bb);
    return __axc_half_log(sum, se + (pe + qe));
}

#include "complex_impl.h"
#else
typedef int __axc_complex_c89;
#endif

/* complexl.c - the long double forms of <complex.h>'s functions: where
   long double is wider than double, complex_impl.h's, from the long double
   functions (std/c/rt/math_ld.ax's precision); where it is double, the
   double forms. */
#if __STDC_VERSION__ >= 199901L
#include <complex.h>
#include <float.h>
#include <math.h>

#if __AXIOM_LDBL_WIDTH__ != 64
#define T long double
#define CT long double complex
#define C(x, y) __builtin_complex((long double)(x), (long double)(y))
#define RE(z) __builtin_creal(z)
#define IM(z) __builtin_cimag(z)
#define INF ((long double)__builtin_inff())
#define FN(n) n##l
#define M(n) n##l
#define K_EXP_BIG 11350.0L
#define K_SQ_HI 8000
#define K_SQ_LO (-8000)
#define K_SQ_HIV 0x1p8000L
#define K_SQ_LOV 0x1p-8000L
#define K_TANH_BIG 45.0L
#define K_ASIN_BIG 0x1p120L
#define K_ATANH_BIG 0x1p8000L
#define K_PIO4 0.785398163397448309615660845819875721L
#define K_PIO2 1.57079632679489661923132169163975144L
#define K_3PIO4 2.35619449019234492884698253745962716L
#define K_PI 3.14159265358979323846264338327950288L
#define K_LN2 0.693147180559945309417232121458176568L
#if __AXIOM_LDBL_WIDTH__ == 80
#define K_LN2_LO (-1.145835272679873281093529986196612039e-20L)
#else
#define K_LN2_LO (-7.008139474549585163412662008771625674e-36L)
#endif

/* the real functions in pairs of long doubles (std/c/rt/math_ld.ax) */
void __axc_ldcx(unsigned long long op, unsigned long long f, long double *r, const long double *x,
                const long double *y);

static void xk1_ld(int op, long double x, long double *r)
{
    __axc_ldcx((unsigned long long)op, __AXIOM_LDBL_WIDTH__ == 80, r, &x, &x);
}

static void xk2_ld(int op, long double y, long double x, long double *r)
{
    __axc_ldcx((unsigned long long)op + 4, __AXIOM_LDBL_WIDTH__ == 80, r, &y, &x);
}
#define XK1(op, x, r) xk1_ld((op), (x), (r))
#define XK2(op, y, x, r) xk2_ld((op), (y), (x), (r))
#define PROD_ERR(a, b, p) fmal((a), (b), -(p))

#include "complex_impl.h"
#else
long double _Complex cacosl(long double _Complex z)
{
    double _Complex w = cacos(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex casinl(long double _Complex z)
{
    double _Complex w = casin(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex catanl(long double _Complex z)
{
    double _Complex w = catan(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex ccosl(long double _Complex z)
{
    double _Complex w = ccos(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex csinl(long double _Complex z)
{
    double _Complex w = csin(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex ctanl(long double _Complex z)
{
    double _Complex w = ctan(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex cacoshl(long double _Complex z)
{
    double _Complex w = cacosh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex casinhl(long double _Complex z)
{
    double _Complex w = casinh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex catanhl(long double _Complex z)
{
    double _Complex w = catanh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex ccoshl(long double _Complex z)
{
    double _Complex w = ccosh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex csinhl(long double _Complex z)
{
    double _Complex w = csinh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex ctanhl(long double _Complex z)
{
    double _Complex w = ctanh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex cexpl(long double _Complex z)
{
    double _Complex w = cexp(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex clogl(long double _Complex z)
{
    double _Complex w = clog(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex csqrtl(long double _Complex z)
{
    double _Complex w = csqrt(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex (conjl)(long double _Complex z)
{
    double _Complex w = conj(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double _Complex cprojl(long double _Complex z)
{
    double _Complex w = cproj(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((long double)__builtin_creal(w), (long double)__builtin_cimag(w));
}

long double cabsl(long double _Complex z)
{
    return (long double)cabs(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
}

long double cargl(long double _Complex z)
{
    return (long double)carg(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
}

long double (cimagl)(long double _Complex z)
{
    return __builtin_cimag(z);
}

long double (creall)(long double _Complex z)
{
    return __builtin_creal(z);
}

long double _Complex cpowl(long double _Complex z, long double _Complex w)
{
    double _Complex r = cpow(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)),
                             __builtin_complex((double)__builtin_creal(w), (double)__builtin_cimag(w)));
    return __builtin_complex((long double)__builtin_creal(r), (long double)__builtin_cimag(r));
}
#endif
#else
typedef int __axc_complexl_c89;
#endif

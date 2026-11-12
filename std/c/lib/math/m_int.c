/* m_int.c - <math.h>'s functions that take or return int or long (C17
   7.12.6.4-7.12.6.13, 7.12.9.5-7, 7.12.10.3, 7.12.11.2-4), whose widths
   depend on the build (--c-int, --c-long): wrappers of the Axiom kernels
   (std/c/rt/math_exact.ax), which take 64-bit integers. lrint and the like
   raise "invalid" and set EDOM for a result outside the return type (F.10.6.5:
   the value is then unspecified; it is the type's minimum). */
#include <errno.h>
#include <limits.h>

#define FE_INVALID_ 0x01

double __axc_frexp(double x, __axiom_int64 *e);
double __axc_scalbn(double x, __axiom_int64 n);
__axiom_int64 __axc_ilogb(double x);
double __axc_remquo(double x, double y, __axiom_int64 *q);
double rint(double x);
double round(double x);
double nextafter(double x, double y);
void __axc_math_err(int e, unsigned int flags);

/* __axc_ilogb's codes (math_exact.ax) */
#define ILOGB_ZERO (-(__axiom_int64)0x10000000000)
#define ILOGB_INF (-(__axiom_int64)0x10000000001)

double frexp(double x, int *e)
{
    __axiom_int64 k;
    double r = __axc_frexp(x, &k);
    *e = (int)k;
    return r;
}

double ldexp(double x, int n)
{
    return __axc_scalbn(x, (__axiom_int64)n);
}

#if __STDC_VERSION__ >= 199901L
/* ilogb's value for 0, infinities and NaNs, which are domain errors */
static int ilogb_special(__axiom_int64 k)
{
    __axc_math_err(EDOM, FE_INVALID_);
    if (k == ILOGB_ZERO)
        return -INT_MAX - 1; /* FP_ILOGB0 */
    return INT_MAX;          /* infinity; NaN: FP_ILOGBNAN */
}

double scalbn(double x, int n)
{
    return __axc_scalbn(x, (__axiom_int64)n);
}

double scalbln(double x, long n)
{
    return __axc_scalbn(x, (__axiom_int64)n);
}

int ilogb(double x)
{
    __axiom_int64 k = __axc_ilogb(x);
    if (k < -100000)
        return ilogb_special(k);
    return (int)k;
}

/* r (an integer or not a number) as a long long or long, or an invalid
   operation: the conversions are those of the range [lo, -lo) */
static long long to_llong(double r)
{
    if (r >= -9223372036854775808.0 && r < 9223372036854775808.0)
        return (long long)r;
    __axc_math_err(EDOM, FE_INVALID_);
    return LLONG_MIN;
}

static long to_long(double r)
{
    if (r >= (double)LONG_MIN && r < -(double)LONG_MIN)
        return (long)r;
    __axc_math_err(EDOM, FE_INVALID_);
    return LONG_MIN;
}

long lrint(double x)
{
    return to_long(rint(x));
}

long long llrint(double x)
{
    return to_llong(rint(x));
}

long lround(double x)
{
    return to_long(round(x));
}

long long llround(double x)
{
    return to_llong(round(x));
}

double remquo(double x, double y, int *q)
{
    __axiom_int64 k;
    double r = __axc_remquo(x, y, &k);
    *q = (int)k;
    return r;
}

double nexttoward(double x, long double y)
{
    if (x != x || y != y)
        return (double)((long double)x + y);
    if ((long double)x == y)
        return (double)y;
    return nextafter(x, (long double)x < y ? (double)__builtin_inff() : -(double)__builtin_inff());
}

/* float */
float __axc_scalbnf(float x, __axiom_int64 n);
float rintf(float x);
float roundf(float x);
float nextafterf(float x, float y);

float frexpf(float x, int *e)
{
    return (float)frexp((double)x, e);
}

float ldexpf(float x, int n)
{
    return __axc_scalbnf(x, (__axiom_int64)n);
}

float scalbnf(float x, int n)
{
    return __axc_scalbnf(x, (__axiom_int64)n);
}

float scalblnf(float x, long n)
{
    return __axc_scalbnf(x, (__axiom_int64)n);
}

int ilogbf(float x)
{
    return ilogb((double)x);
}

long lrintf(float x)
{
    return to_long((double)rintf(x));
}

long long llrintf(float x)
{
    return to_llong((double)rintf(x));
}

long lroundf(float x)
{
    return to_long((double)roundf(x));
}

long long llroundf(float x)
{
    return to_llong((double)roundf(x));
}

float remquof(float x, float y, int *q)
{
    return (float)remquo((double)x, (double)y, q);
}

float nexttowardf(float x, long double y)
{
    if (x != x || y != y)
        return (float)((long double)x + y);
    if ((long double)x == y)
        return (float)y;
    return nextafterf(x, (long double)x < y ? __builtin_inff() : -__builtin_inff());
}
#endif

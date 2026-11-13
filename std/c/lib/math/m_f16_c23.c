/* m_f16_c23.c - the _Float16 forms of the functions C23 adds to <math.h>
   (C23 Annex H.11), binary16. Those that compute (exp2m1f16 ...
   compoundnf16) are correctly rounded as m_f16.c's elementary functions
   are: the double function's value rounded once to binary16 where both ends
   of its error interval round alike, else the _Float128 function's
   (m_f128_c23.c); with the range errors of binary16 (ERANGE for a result
   that overflows it, or is subnormal or zero from a nonzero value). Those
   that work on the representation do it on binary16's bits: the payload is
   the 9 fraction bits below the quiet bit, total order on the encodings,
   every encoding canonical (a signaling NaN made quiet, with "invalid");
   fromfp and its kin are float's (whose values hold binary16's). */
#if __STDC_VERSION__ >= 199901L
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <errno.h>
#include <limits.h>
#include <math.h>

void __axc_math_err(int e, unsigned int flags);

#define FE_INVALID_ 0x01
#define FE_OVERFLOW_ 0x04
#define FE_UNDERFLOW_ 0x08
#define FE_INEXACT_ 0x20

/* C23's double and float functions (declared by <math.h> in C23 only) */
double exp2m1(double), exp10m1(double), log2p1(double), log10p1(double), logp1(double), rsqrt(double);
double sinpi(double), cospi(double), tanpi(double), asinpi(double), acospi(double), atanpi(double);
double atan2pi(double, double), rootn(double, long long), pown(double, long long), powr(double, double);
double compoundn(double, long long);
float fromfpf(float, int, unsigned int), ufromfpf(float, int, unsigned int);
float fromfpxf(float, int, unsigned int), ufromfpxf(float, int, unsigned int);
_Float128 exp2m1f128(_Float128), exp10m1f128(_Float128), log2p1f128(_Float128), log10p1f128(_Float128);
_Float128 logp1f128(_Float128), rsqrtf128(_Float128), sinpif128(_Float128), cospif128(_Float128);
_Float128 tanpif128(_Float128), asinpif128(_Float128), acospif128(_Float128), atanpif128(_Float128);
_Float128 atan2pif128(_Float128, _Float128), rootnf128(_Float128, long long), pownf128(_Float128, long long);
_Float128 powrf128(_Float128, _Float128), compoundnf128(_Float128, long long);

/* r rounds to binary16 alike at both ends of its error interval */
static int sure(double r)
{
    return (_Float16)(r * (1.0 - 0x1p-50)) == (_Float16)(r * (1.0 + 0x1p-50)) || r != r;
}

/* v, the result of a nonzero finite value r: binary16's range errors */
static _Float16 ranged(_Float16 v, double r)
{
    if (r == r && r - r == 0.0) {
        if (v - v != 0)
            errno = ERANGE;
        else if (r != 0.0 && (v == 0 || (v < 0x1p-14F16 && v > -0x1p-14F16)))
            errno = ERANGE;
    }
    return v;
}

#define E1(name) \
    _Float16 name##f16(_Float16 x) \
    { \
        double r = name((double)x); \
        if (sure(r)) \
            return ranged((_Float16)r, r); \
        return ranged((_Float16)name##f128((_Float128)x), r); \
    }
E1(exp2m1) E1(exp10m1) E1(log2p1) E1(log10p1) E1(logp1) E1(rsqrt) E1(sinpi) E1(cospi) E1(tanpi) E1(asinpi)
E1(acospi) E1(atanpi)

_Float16 atan2pif16(_Float16 y, _Float16 x)
{
    double r = atan2pi((double)y, (double)x);
    if (sure(r))
        return ranged((_Float16)r, r);
    return ranged((_Float16)atan2pif128((_Float128)y, (_Float128)x), r);
}

_Float16 powrf16(_Float16 x, _Float16 y)
{
    double r = powr((double)x, (double)y);
    if (sure(r))
        return ranged((_Float16)r, r);
    return ranged((_Float16)powrf128((_Float128)x, (_Float128)y), r);
}

#define EN(name) \
    _Float16 name##f16(_Float16 x, long long n) \
    { \
        double r = name((double)x, n); \
        if (sure(r)) \
            return ranged((_Float16)r, r); \
        return ranged((_Float16)name##f128((_Float128)x, n), r); \
    }
EN(rootn) EN(pown) EN(compoundn)

_Float16 fromfpf16(_Float16 x, int rnd, unsigned int width) { return (_Float16)fromfpf((float)x, rnd, width); }
_Float16 ufromfpf16(_Float16 x, int rnd, unsigned int width) { return (_Float16)ufromfpf((float)x, rnd, width); }
_Float16 fromfpxf16(_Float16 x, int rnd, unsigned int width) { return (_Float16)fromfpxf((float)x, rnd, width); }
_Float16 ufromfpxf16(_Float16 x, int rnd, unsigned int width) { return (_Float16)ufromfpxf((float)x, rnd, width); }

long llogbf16(_Float16 x)
{
    if (x == 0 || x != x || x - x != 0) {
        __axc_math_err(EDOM, FE_INVALID_);
        return x == 0 ? -LONG_MAX - 1 : LONG_MAX;
    }
    return (long)ilogbf((float)x);
}

static unsigned bits_of(_Float16 x)
{
    union {
        _Float16 v;
        unsigned short b;
    } u;
    u.v = x;
    return u.b;
}

static _Float16 of_bits(unsigned b)
{
    union {
        _Float16 v;
        unsigned short b;
    } u;
    u.b = (unsigned short)b;
    return u.v;
}

_Float16 getpayloadf16(const _Float16 *x)
{
    if (*x == *x)
        return -1;
    return (_Float16)(bits_of(*x) & 0x1ffu);
}

static int setpl_h(_Float16 *res, _Float16 pl, int sig)
{
    float p = (float)pl;
    if (!(p >= 0.0f) || p != truncf(p) || p >= 512.0f || (sig && p == 0.0f)) {
        *res = 0;
        return 1;
    }
    *res = of_bits(0x7c00u | (sig ? 0u : 0x200u) | (unsigned)p);
    return 0;
}

int setpayloadf16(_Float16 *res, _Float16 pl) { return setpl_h(res, pl, 0); }
int setpayloadsigf16(_Float16 *res, _Float16 pl) { return setpl_h(res, pl, 1); }

/* the encodings as integers that order as the values, negatives reversed */
static unsigned key_h(unsigned u)
{
    return (u & 0x8000u) ? 0x7fffu - (u & 0x7fffu) : 0x8000u + u;
}

int totalorderf16(const _Float16 *x, const _Float16 *y)
{
    return key_h(bits_of(*x)) <= key_h(bits_of(*y));
}

int totalordermagf16(const _Float16 *x, const _Float16 *y)
{
    return (bits_of(*x) & 0x7fffu) <= (bits_of(*y) & 0x7fffu);
}

int canonicalizef16(_Float16 *cx, const _Float16 *x)
{
    unsigned b = bits_of(*x);
    if ((b & 0x7c00u) == 0x7c00u && (b & 0x3ffu) != 0 && (b & 0x200u) == 0) {
        __axc_math_err(0, FE_INVALID_);
        b |= 0x200u;
    }
    *cx = of_bits(b);
    return 0;
}
#else
typedef int __axc_f16_c23_c89;
#endif

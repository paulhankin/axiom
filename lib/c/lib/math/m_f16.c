/* m_f16.c - <math.h>'s _Float16 functions (ISO/IEC TS 18661-3, C23 Annex
   H: the f16 suffix), binary16. The exact ones (fabs, floor, fmod, frexp,
   ...) are float's, whose results are values of the argument's format;
   sqrt and fdim are float's rounded (float has more than twice binary16's
   precision and two more bits, so the double rounding is innocuous); fma
   computes in _Float128, where the product and the sum are exact. The
   elementary functions are correctly rounded: the double function's value
   r (within 2^-51 of the exact one) rounded once to binary16, unless both
   ends of [r (1 - 2^-50), r (1 + 2^-50)] do not round alike (the exact
   value may then be on either side of a rounding boundary), where the
   _Float128 function's value (within 2^-117: math_ld.ax) is rounded
   instead; checked against an arbitrary-precision reference for every
   binary16 argument of the one-argument functions. nextafter and its kin
   step binary16's bits. */
#if __STDC_VERSION__ >= 199901L
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <math.h>

/* r rounds to binary16 alike at both ends of its error interval */
static int sure(double r)
{
    return (_Float16)(r * (1.0 - 0x1p-50)) == (_Float16)(r * (1.0 + 0x1p-50)) || r != r;
}

#define H1(name) \
    _Float16 name##f16(_Float16 x) { return (_Float16)name##f((float)x); }
#define H2(name) \
    _Float16 name##f16(_Float16 x, _Float16 y) { return (_Float16)name##f((float)x, (float)y); }
#define E1(name) \
    _Float16 name##f16(_Float16 x) \
    { \
        double r = name((double)x); \
        if (sure(r)) \
            return (_Float16)r; \
        return (_Float16)name##f128((_Float128)x); \
    }
#define E2(name) \
    _Float16 name##f16(_Float16 x, _Float16 y) \
    { \
        double r = name((double)x, (double)y); \
        if (sure(r)) \
            return (_Float16)r; \
        return (_Float16)name##f128((_Float128)x, (_Float128)y); \
    }
E1(acos) E1(asin) E1(atan) E1(cos) E1(sin) E1(tan) E1(acosh) E1(asinh) E1(atanh) E1(cosh) E1(sinh) E1(tanh)
E1(exp) E1(exp2) E1(exp10) E1(expm1) E1(log) E1(log10) E1(log1p) E1(log2) E1(cbrt) E1(erf) E1(erfc) E1(lgamma)
E1(tgamma)
E2(atan2) E2(pow) E2(hypot)
H1(logb) H1(sqrt) H1(ceil) H1(floor) H1(nearbyint) H1(rint) H1(round) H1(roundeven) H1(trunc) H1(fabs)
H2(fmod) H2(remainder) H2(copysign) H2(fdim) H2(fmax) H2(fmin) H2(fmaximum)
H2(fminimum) H2(fmaximum_mag) H2(fminimum_mag) H2(fmaximum_num) H2(fminimum_num) H2(fmaximum_mag_num)
H2(fminimum_mag_num)

_Float16 fmaf16(_Float16 x, _Float16 y, _Float16 z)
{
    return (_Float16)((_Float128)x * (_Float128)y + (_Float128)z);
}
_Float16 frexpf16(_Float16 x, int *e) { return (_Float16)frexpf((float)x, e); }
_Float16 ldexpf16(_Float16 x, int n) { return (_Float16)ldexpf((float)x, n); }
_Float16 scalbnf16(_Float16 x, int n) { return (_Float16)scalbnf((float)x, n); }
_Float16 scalblnf16(_Float16 x, long n) { return (_Float16)scalblnf((float)x, n); }
int ilogbf16(_Float16 x) { return ilogbf((float)x); }
long lrintf16(_Float16 x) { return lrintf((float)x); }
long long llrintf16(_Float16 x) { return llrintf((float)x); }
long lroundf16(_Float16 x) { return lroundf((float)x); }
long long llroundf16(_Float16 x) { return llroundf((float)x); }
_Float16 remquof16(_Float16 x, _Float16 y, int *q) { return (_Float16)remquof((float)x, (float)y, q); }
_Float16 modff16(_Float16 x, _Float16 *ip)
{
    float i;
    float r = modff((float)x, &i);
    *ip = (_Float16)i;
    return (_Float16)r;
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

/* the next value of binary16 from x toward +infinity (up) or -infinity */
static _Float16 step(_Float16 x, int up)
{
    unsigned b = bits_of(x);
    if (x != x || (up && b == 0x7c00u) || (!up && b == 0xfc00u))
        return x;
    if ((b & 0x7fffu) == 0)
        return of_bits(up ? 1u : 0x8001u);
    if (((b >> 15) == 0) == (up != 0))
        return of_bits(b + 1);
    return of_bits(b - 1);
}

_Float16 nextupf16(_Float16 x) { return step(x, 1); }
_Float16 nextdownf16(_Float16 x) { return step(x, 0); }
_Float16 nextafterf16(_Float16 x, _Float16 y)
{
    _Float16 r;
    if (x != x || y != y)
        return x + y;
    if (x == y)
        return y;
    r = step(x, y > x);
    /* overflow, or a subnormal or zero result: the exceptions */
    if ((bits_of(r) & 0x7c00u) == 0x7c00u || (bits_of(r) & 0x7c00u) == 0) {
        volatile float t = (float)r * (float)r;
        (void)t;
    }
    return r;
}
#else
typedef int __axc_f16_c89;
#endif

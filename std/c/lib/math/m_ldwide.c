/* m_ldwide.c - <math.h>'s exact long double functions where long double is
   wider than double (the x87's 80-bit format: x86-64 Linux; IEEE 754
   binary128: AArch64 Linux, WebAssembly): computed on the format's bits and
   with its arithmetic (correctly rounded to nearest, in software:
   std/c/rt/ldouble.ax), so that they are exact (and sqrtl correctly
   rounded). The elementary l functions compute in double (m_ldouble.c).

   The functions whose results depend on the rounding direction (F.10:
   rint, nearbyint, lrint, llrint, sqrt, scalbn and its kin, fdim) read the
   current one (__axc_fe_round) and raise their exceptions: their own
   arithmetic is to nearest (this file is not under FENV_ACCESS ON, but for
   the steps that must round in the direction, which are), so the direction
   is applied at the end.

   The formats (little-endian): x87, a 64-bit significand with its integer
   bit (bytes 0-7), then the sign and a 15-bit exponent (bytes 8-9), six
   bytes of padding; binary128, a 112-bit fraction (bytes 0-13, the low 64
   bits first), the exponent and the sign in bytes 14-15. Both have the
   bias 16383. Where long double is double, these are the double
   functions. */
#if __STDC_VERSION__ >= 199901L
/* (the long double instance; m_f128.c defines LDT and the others first
   for its _Float128 one) */
#if !defined(LDT) && __AXIOM_LDBL_WIDTH__ != 64
#define LDT long double
#define FN(name) name##l
#define K(c) c##L
#define X87 (__AXIOM_LDBL_WIDTH__ == 80)
#define P LDBL_MANT_DIG /* 64 or 113 */
#define LD_NEXTTOWARD 1
#endif
#ifdef LDT
#include <errno.h>
#include <float.h>
#include <limits.h>

#define BIAS 16383

void __axc_math_err(int e, unsigned int flags);
int __axc_fe_round(void);
double sqrt(double);

/* the current rounding direction: 0 to nearest, 1 downward, 2 upward, 3
   toward zero (fenv.h's FE_ values are these times 0x400) */
static int dir(void)
{
    return (__axc_fe_round() >> 10) & 3;
}

typedef union {
    LDT v;
    unsigned char b[16];
} LD;

/* the significand, as a 128-bit integer (hi, lo): x87's 64 bits in lo with
   the integer bit, binary128's fraction with the implicit bit set for
   normal numbers */
typedef struct {
    int neg;
    int e; /* the biased exponent field */
    unsigned long long hi, lo;
} Parts;

static unsigned long long get64(const unsigned char *p)
{
    unsigned long long v = 0;
    int i;
    for (i = 7; i >= 0; i--)
        v = (v << 8) | p[i];
    return v;
}

static void put64(unsigned char *p, unsigned long long v)
{
    int i;
    for (i = 0; i < 8; i++) {
        p[i] = (unsigned char)(v & 0xff);
        v >>= 8;
    }
}

static Parts parts(LDT x)
{
    LD u;
    Parts r;
    unsigned se;
    u.v = x;
    if (X87) {
        se = (unsigned)u.b[8] | ((unsigned)u.b[9] << 8);
        r.lo = get64(u.b);
        r.hi = 0;
    } else {
        unsigned long long h = get64(u.b + 8);
        se = (unsigned)(h >> 48);
        r.lo = get64(u.b);
        r.hi = h & 0xffffffffffffULL;
    }
    r.neg = (se >> 15) & 1;
    r.e = (int)(se & 0x7fff);
    return r;
}

static LDT make(Parts r)
{
    LD u;
    unsigned se = ((unsigned)r.neg << 15) | (unsigned)r.e;
    int i;
    for (i = 0; i < 16; i++)
        u.b[i] = 0;
    put64(u.b, r.lo);
    if (X87) {
        u.b[8] = (unsigned char)(se & 0xff);
        u.b[9] = (unsigned char)(se >> 8);
    } else {
        put64(u.b + 8, (r.hi & 0xffffffffffffULL) | ((unsigned long long)se << 48));
    }
    return u.v;
}

static int is_nan(LDT x)
{
    return x != x;
}

/* x + x and x + y in the environment: a NaN operand quieted, with "invalid"
   for a signaling one (the results of the functions for NaNs, F.10p11) */
static LDT env_add(LDT x, LDT y)
{
#pragma STDC FENV_ACCESS ON
    return x + y;
}

static LDT inf_l(void)
{
    Parts r;
    r.neg = 0;
    r.e = 0x7fff;
    r.hi = 0;
    r.lo = X87 ? 0x8000000000000000ULL : 0;
    return make(r);
}

/* 2^n, for -16382 <= n <= 16383 */
static LDT pow2l(int n)
{
    Parts r;
    r.neg = 0;
    r.e = n + BIAS;
    r.hi = 0;
    r.lo = X87 ? 0x8000000000000000ULL : 0;
    return make(r);
}

LDT FN(fabs)(LDT x)
{
    Parts r = parts(x);
    r.neg = 0;
    return make(r);
}

LDT FN(copysign)(LDT x, LDT y)
{
    Parts r = parts(x);
    r.neg = parts(y).neg;
    return make(r);
}

/* x with its fraction bits below the units cleared (|x| >= 1, finite) */
static LDT trunc_bits(LDT x)
{
    Parts r = parts(x);
    int k = r.e - BIAS;          /* the units bit is k bits below the leading one */
    int drop = (P - 1) - k;      /* fraction bits to clear */
    if (drop <= 0)
        return x;
    if (drop >= 64) {
        r.lo = 0;
        r.hi &= ~((1ULL << (drop - 64)) - 1);
    } else {
        r.lo &= ~((1ULL << drop) - 1);
    }
    return make(r);
}

LDT FN(trunc)(LDT x)
{
    Parts r = parts(x);
    if (r.e == 0x7fff)
        return env_add(x, x); /* infinities, NaNs */
    if (r.e >= BIAS + P - 1)
        return x; /* integers */
    if (r.e < BIAS)
        return FN(copysign)(K(0.0), x); /* |x| < 1 */
    return trunc_bits(x);
}

LDT FN(floor)(LDT x)
{
    LDT t = FN(trunc)(x);
    if (x < t)
        return t - K(1.0);
    return t;
}

LDT FN(ceil)(LDT x)
{
    LDT t = FN(trunc)(x);
    if (x > t)
        return t + K(1.0);
    return t;
}

LDT FN(round)(LDT x)
{
    LDT t = FN(trunc)(x);
    if (FN(fabs)(x - t) >= K(0.5))
        return t + FN(copysign)(K(1.0), x);
    return t;
}

/* t (an integer) is odd */
static int odd(LDT t)
{
    Parts r = parts(t);
    int k = r.e - BIAS, drop = (P - 1) - k;
    if (r.e < BIAS || drop < 0)
        return 0;
    if (drop >= 64)
        return (int)((r.hi >> (drop - 64)) & 1);
    return (int)((r.lo >> drop) & 1);
}

/* x rounded to an integer in direction m (dir's numbers): "inexact" if
   that changed it and raise (F.10.6.4, F.10.6.5) */
static LDT round_int(LDT x, int m, int raise)
{
    LDT t = FN(trunc)(x), d;
    int up;
    if (is_nan(x))
        return env_add(x, x);
    if (t == x)
        return x; /* (integers, infinities and zeros, exactly) */
    d = FN(fabs)(x - t);
    if (m == 0)
        up = d > K(0.5) || (d == K(0.5) && odd(t));
    else if (m == 1)
        up = x < 0;
    else if (m == 2)
        up = x > 0;
    else
        up = 0;
    if (raise)
        __axc_math_err(0, 0x20);
    if (up)
        return t + FN(copysign)(K(1.0), x);
    return t;
}

LDT FN(rint)(LDT x)
{
    return round_int(x, dir(), 1);
}

LDT FN(nearbyint)(LDT x)
{
    return round_int(x, dir(), 0);
}

LDT FN(roundeven)(LDT x)
{
    return round_int(x, 0, 0);
}

static long long to_ll(LDT r)
{
    if (r >= -K(0x1p63) && r < K(0x1p63))
        return (long long)r;
    __axc_math_err(EDOM, 0x01);
    return LLONG_MIN;
}

static long to_l(LDT r)
{
    if (r >= (LDT)LONG_MIN && r < -(LDT)LONG_MIN)
        return (long)r;
    __axc_math_err(EDOM, 0x01);
    return LONG_MIN;
}

/* (out of range only "invalid", else "inexact" if x is not an integer:
   IEEE 754-2019 5.8) */
long FN(lrint)(LDT x)
{
    LDT r = round_int(x, dir(), 0);
    if (r != x && r >= (LDT)LONG_MIN && r < -(LDT)LONG_MIN)
        __axc_math_err(0, 0x20);
    return to_l(r);
}

long long FN(llrint)(LDT x)
{
    LDT r = round_int(x, dir(), 0);
    if (r != x && r >= -K(0x1p63) && r < K(0x1p63))
        __axc_math_err(0, 0x20);
    return to_ll(r);
}

long FN(lround)(LDT x)
{
    return to_l(FN(round)(x));
}

long long FN(llround)(LDT x)
{
    return to_ll(FN(round)(x));
}

LDT FN(modf)(LDT x, LDT *ip)
{
    LDT t = FN(trunc)(x);
    *ip = t;
    if (parts(x).e == 0x7fff && !is_nan(x))
        return FN(copysign)(K(0.0), x);
    return FN(copysign)(x - t, x);
}

/* x 2^n, rounded once (in steps that are exact while the result is normal),
   in the current direction (the steps under FENV_ACCESS ON: a subnormal or
   overflowing result rounds as the direction says, with its exceptions) */
static LDT scale(LDT x, long n)
{
#pragma STDC FENV_ACCESS ON
    if (n > 16383) {
        x *= pow2l(16383);
        n -= 16383;
        if (n > 16383) {
            x *= pow2l(16383);
            n -= 16383;
            if (n > 16383)
                n = 16383;
        }
    } else if (n < -16382) {
        x *= pow2l(-16382 + P);
        n += 16382 - P;
        if (n < -16382) {
            x *= pow2l(-16382 + P);
            n += 16382 - P;
            if (n < -16382)
                n = -16382;
        }
    }
    return x * pow2l((int)n);
}

int FN(ilogb)(LDT x);

/* x 2^n to nearest (exact for a subnormal x scaled up) */
static LDT unscale(LDT x, long n)
{
    while (n > 16383) {
        x *= pow2l(16383);
        n -= 16383;
    }
    return x * pow2l((int)n);
}

LDT FN(scalbln)(LDT x, long n)
{
    LDT r;
    if (parts(x).e == 0x7fff)
        return env_add(x, x);
    if (x == 0 || n == 0)
        return x;
    r = scale(x, n);
    /* (an overflow: the result's exponent beyond the format's, whatever the
       direction made of it; an underflow: a subnormal or zero result that
       is not exact) */
    if ((long)FN(ilogb)(x) + n > 16383 || ((parts(r).e == 0) && unscale(r, -n) != x))
        __axc_math_err(ERANGE, 0);
    return r;
}

LDT FN(scalbn)(LDT x, int n)
{
    return FN(scalbln)(x, (long)n);
}

LDT FN(ldexp)(LDT x, int n)
{
    return FN(scalbln)(x, (long)n);
}

LDT FN(frexp)(LDT x, int *ep)
{
    Parts r = parts(x);
    int adj = 0;
    if (x == 0 || r.e == 0x7fff) {
        *ep = 0;
        return env_add(x, x);
    }
    if (r.e == 0) {
        x *= pow2l(P);
        adj = -P;
        r = parts(x);
    }
    *ep = r.e - (BIAS - 1) + adj;
    r.e = BIAS - 1;
    return make(r);
}

int FN(ilogb)(LDT x)
{
    Parts r = parts(x);
    if (x == 0 || r.e == 0x7fff) {
        __axc_math_err(EDOM, 0x01);
        if (x == 0)
            return -INT_MAX - 1;
        return INT_MAX;
    }
    if (r.e == 0)
        return parts(x * pow2l(P)).e - BIAS - P;
    return r.e - BIAS;
}

LDT FN(logb)(LDT x)
{
    Parts r = parts(x);
    if (x == 0) {
        volatile LDT z = K(0.0);
        __axc_math_err(ERANGE, 0x04);
        return -K(1.0) / z;
    }
    if (r.e == 0x7fff)
        return x * x;
    return (LDT)FN(ilogb)(x);
}

/* |x| mod |y| by subtracting |y| 2^k (exactly: Sterbenz's lemma), from the
   largest k down; the quotient's low bits in *q */
static LDT mod_core(LDT ax, LDT ay, unsigned *q)
{
    int k = FN(ilogb)(ax) - FN(ilogb)(ay);
    unsigned qq = 0;
    while (k >= 0) {
        LDT t = scale(ay, k);
        qq <<= 1;
        if (ax >= t) {
            ax -= t;
            qq |= 1;
        }
        k--;
    }
    *q = qq;
    return ax;
}

LDT FN(fmod)(LDT x, LDT y)
{
    unsigned q;
    LDT r;
    if (is_nan(x) || is_nan(y))
        return env_add(x, y);
    if (parts(x).e == 0x7fff || y == 0) {
        volatile LDT z = K(0.0);
        __axc_math_err(EDOM, 0x01);
        return z / z;
    }
    if (FN(fabs)(x) < FN(fabs)(y))
        return x;
    r = mod_core(FN(fabs)(x), FN(fabs)(y), &q);
    return FN(copysign)(r, x);
}

LDT FN(remquo)(LDT x, LDT y, int *quo)
{
    unsigned q = 0;
    LDT ax = FN(fabs)(x), ay = FN(fabs)(y), r;
    int neg = parts(x).neg ^ parts(y).neg;
    *quo = 0;
    if (is_nan(x) || is_nan(y))
        return env_add(x, y);
    if (parts(x).e == 0x7fff || y == 0) {
        volatile LDT z = K(0.0);
        __axc_math_err(EDOM, 0x01);
        return z / z;
    }
    if (parts(y).e == 0x7fff)
        return x;
    r = ax >= ay ? mod_core(ax, ay, &q) : ax;
    /* n = q or q + 1: the nearer, the even one in a tie (r against |y|/2,
       |y| - r being exact) */
    if (r > ay - r || (r == ay - r && (q & 1))) {
        r = r - ay;
        q++;
    }
    *quo = (int)(q & 7) * (neg ? -1 : 1);
    return parts(x).neg ? -r : r;
}

LDT FN(remainder)(LDT x, LDT y)
{
    int q;
    return FN(remquo)(x, y, &q);
}

/* the next value of |x| up (dir > 0) or down, by the significand's bits */
static LDT step(LDT x, int up)
{
    Parts r = parts(x);
    if (X87) {
        if (up) {
            r.lo++;
            if (r.lo == 0) {
                r.lo = 0x8000000000000000ULL;
                r.e++;
            } else if (r.e == 0 && r.lo == 0x8000000000000000ULL) {
                r.e = 1;
            }
        } else {
            if (r.e >= 1 && r.lo == 0x8000000000000000ULL) {
                r.e--;
                r.lo = r.e == 0 ? 0x7fffffffffffffffULL : 0xffffffffffffffffULL;
            } else {
                r.lo--;
            }
        }
        if (r.e == 0x7fff)
            r.lo = 0x8000000000000000ULL;
        return make(r);
    }
    /* binary128: the exponent and fraction are one 127-bit integer */
    if (up) {
        r.lo++;
        if (r.lo == 0) {
            r.hi++;
            if (r.hi >> 48) {
                r.hi = 0;
                r.e++;
            }
        }
    } else {
        if (r.lo == 0) {
            if (r.hi == 0) {
                r.hi = 0xffffffffffffULL;
                r.e--;
            } else {
                r.hi--;
            }
        }
        r.lo--;
    }
    return make(r);
}

LDT FN(nextafter)(LDT x, LDT y)
{
    LDT r;
    if (is_nan(x) || is_nan(y))
        return env_add(x, y);
    if (x == y)
        return y;
    if (x == 0) {
        Parts t;
        t.neg = y < 0;
        t.e = 0;
        t.hi = 0;
        t.lo = 1;
        r = make(t);
    } else {
        r = step(x, (x < y) == (x > 0));
    }
    if (parts(r).e == 0x7fff) {
        __axc_math_err(ERANGE, 0x08 | 0x20);
    } else if (parts(r).e == 0) {
        __axc_math_err(ERANGE, 0x10 | 0x20);
    }
    return r;
}

#if LD_NEXTTOWARD
LDT FN(nexttoward)(LDT x, LDT y)
{
    return FN(nextafter)(x, y);
}
#endif

LDT FN(nextup)(LDT x)
{
    if (is_nan(x) || x == inf_l())
        return env_add(x, x);
    return FN(nextafter)(x, inf_l());
}

LDT FN(nextdown)(LDT x)
{
    if (is_nan(x) || x == -inf_l())
        return env_add(x, x);
    return FN(nextafter)(x, -inf_l());
}

LDT FN(fmax)(LDT x, LDT y)
{
    if (is_nan(x))
        return y;
    if (is_nan(y))
        return x;
    if (x == y)
        return parts(x).neg ? y : x;
    return x > y ? x : y;
}

LDT FN(fmin)(LDT x, LDT y)
{
    if (is_nan(x))
        return y;
    if (is_nan(y))
        return x;
    if (x == y)
        return parts(x).neg ? x : y;
    return x < y ? x : y;
}

LDT FN(fdim)(LDT x, LDT y)
{
#pragma STDC FENV_ACCESS ON
    if (is_nan(x) || is_nan(y))
        return env_add(x, y);
    return x > y ? x - y : K(0.0);
}

/* a = h + l, each with at most half the significand's bits (Veltkamp) */
static void split(LDT a, LDT *h, LDT *l)
{
    const LDT c = X87 ? K(4294967297.0) : K(144115188075855873.0); /* 2^32 + 1, 2^57 + 1 */
    LDT t = c * a;
    *h = t - (t - a);
    *l = a - *h;
}

/* x - y^2, exactly enough for its sign and size (Dekker's product) */
static LDT residual(LDT x, LDT y)
{
    LDT yh, yl, p = y * y, e;
    split(y, &yh, &yl);
    e = ((yh * yh - p) + K(2.0) * yh * yl) + yl * yl;
    return (x - p) - e;
}

/* The square root, correctly rounded: an estimate from double's, Newton's
   steps in LDT, then the neighbour whose interval holds sqrt(x):
   x against the squares of the midpoints, through the exact residual. */
LDT FN(sqrt)(LDT x)
{
    LDT y, d, u;
    int k;
    if (is_nan(x) || x == 0 || x == inf_l())
        return env_add(x, x);
    if (x < 0) {
        volatile LDT z = K(0.0);
        __axc_math_err(EDOM, 0x01);
        return z / z;
    }
    /* x = m 4^k, m in [1, 4) */
    k = FN(ilogb)(x) >> 1;
    {
        LDT m = scale(x, -2L * k);
        y = (LDT)sqrt((double)m);
        y = K(0.5) * (y + m / y);
        y = K(0.5) * (y + m / y);
        y = K(0.5) * (y + m / y);
        /* the nearer of y and its neighbours: sqrt(m) >= y + u/2 when
           m - y^2 >= y u + u^2/4 */
        d = residual(m, y);
        u = step(y, 1) - y;
        if (d - y * u >= K(0.25) * u * u)
            y = step(y, 1);
        else {
            u = y - step(y, 0);
            if (d + y * u < K(0.25) * u * u)
                y = step(y, 0);
        }
        /* y is sqrt(m) rounded to nearest; the residual's sign says on
           which side of it sqrt(m) is, for the other directions (and
           whether the result is exact) */
        d = residual(m, y);
        if (d != 0) {
            int r = dir();
            if (d < 0 && (r == 1 || r == 3))
                y = step(y, 0);
            else if (d > 0 && r == 2)
                y = step(y, 1);
            __axc_math_err(0, 0x20);
        }
        return unscale(y, k);
    }
}
/* C23's maximum and minimum (IEEE 754-2019's) */
LDT FN(fmaximum)(LDT x, LDT y)
{
    return is_nan(x) || is_nan(y) ? env_add(x, y) : FN(fmax)(x, y);
}

LDT FN(fminimum)(LDT x, LDT y)
{
    return is_nan(x) || is_nan(y) ? env_add(x, y) : FN(fmin)(x, y);
}

LDT FN(fmaximum_mag)(LDT x, LDT y)
{
    if (is_nan(x) || is_nan(y))
        return env_add(x, y);
    if (FN(fabs)(x) != FN(fabs)(y))
        return FN(fabs)(x) > FN(fabs)(y) ? x : y;
    return FN(fmax)(x, y);
}

LDT FN(fminimum_mag)(LDT x, LDT y)
{
    if (is_nan(x) || is_nan(y))
        return env_add(x, y);
    if (FN(fabs)(x) != FN(fabs)(y))
        return FN(fabs)(x) < FN(fabs)(y) ? x : y;
    return FN(fmin)(x, y);
}

LDT FN(fmaximum_num)(LDT x, LDT y)
{
    return FN(fmax)(x, y);
}

LDT FN(fminimum_num)(LDT x, LDT y)
{
    return FN(fmin)(x, y);
}

LDT FN(fmaximum_mag_num)(LDT x, LDT y)
{
    return is_nan(x) ? y : is_nan(y) ? x : FN(fmaximum_mag)(x, y);
}

LDT FN(fminimum_mag_num)(LDT x, LDT y)
{
    return is_nan(x) ? y : is_nan(y) ? x : FN(fminimum_mag)(x, y);
}
#else
/* where long double is double: the double functions (the same names in
   every format: the index names this file for them) */
/* (the exact functions, where long double is double) */
double sqrt(double);
long double sqrtl(long double x) { return (long double)sqrt((double)x); }
double ceil(double);
long double ceill(long double x) { return (long double)ceil((double)x); }
double fabs(double);
long double fabsl(long double x) { return (long double)fabs((double)x); }
double floor(double);
long double floorl(long double x) { return (long double)floor((double)x); }
double nearbyint(double);
long double nearbyintl(long double x) { return (long double)nearbyint((double)x); }
double rint(double);
long double rintl(long double x) { return (long double)rint((double)x); }
double round(double);
long double roundl(long double x) { return (long double)round((double)x); }
double trunc(double);
long double truncl(long double x) { return (long double)trunc((double)x); }
double logb(double);
long double logbl(long double x) { return (long double)logb((double)x); }
double fmod(double, double);
long double fmodl(long double x, long double y) { return (long double)fmod((double)x, (double)y); }
double remainder(double, double);
long double remainderl(long double x, long double y) { return (long double)remainder((double)x, (double)y); }
double copysign(double, double);
long double copysignl(long double x, long double y) { return (long double)copysign((double)x, (double)y); }
double nextafter(double, double);
long double nextafterl(long double x, long double y) { return (long double)nextafter((double)x, (double)y); }
double fdim(double, double);
long double fdiml(long double x, long double y) { return (long double)fdim((double)x, (double)y); }
double fmax(double, double);
long double fmaxl(long double x, long double y) { return (long double)fmax((double)x, (double)y); }
double fmin(double, double);
long double fminl(long double x, long double y) { return (long double)fmin((double)x, (double)y); }
double frexp(double, int *);
long double frexpl(long double x, int *e) { return (long double)frexp((double)x, e); }
double ldexp(double, int);
long double ldexpl(long double x, int n) { return (long double)ldexp((double)x, n); }
double scalbn(double, int);
long double scalbnl(long double x, int n) { return (long double)scalbn((double)x, n); }
double scalbln(double, long);
long double scalblnl(long double x, long n) { return (long double)scalbln((double)x, n); }
int ilogb(double);
int ilogbl(long double x) { return ilogb((double)x); }
long lrint(double);
long lrintl(long double x) { return lrint((double)x); }
long long llrint(double);
long long llrintl(long double x) { return llrint((double)x); }
long lround(double);
long lroundl(long double x) { return lround((double)x); }
long long llround(double);
long long llroundl(long double x) { return llround((double)x); }
double remquo(double, double, int *);
long double remquol(long double x, long double y, int *q) { return (long double)remquo((double)x, (double)y, q); }
double modf(double, double *);
long double modfl(long double x, long double *ip)
{
    double i;
    double r = modf((double)x, &i);
    *ip = (long double)i;
    return (long double)r;
}
double nexttoward(double, long double);
long double nexttowardl(long double x, long double y) { return (long double)nexttoward((double)x, y); }
double roundeven(double);
long double roundevenl(long double x) { return (long double)roundeven((double)x); }
double nextup(double);
long double nextupl(long double x) { return (long double)nextup((double)x); }
double nextdown(double);
long double nextdownl(long double x) { return (long double)nextdown((double)x); }
double fmaximum(double, double);
long double fmaximuml(long double x, long double y) { return (long double)fmaximum((double)x, (double)y); }
double fminimum(double, double);
long double fminimuml(long double x, long double y) { return (long double)fminimum((double)x, (double)y); }
double fmaximum_mag(double, double);
long double fmaximum_magl(long double x, long double y) { return (long double)fmaximum_mag((double)x, (double)y); }
double fminimum_mag(double, double);
long double fminimum_magl(long double x, long double y) { return (long double)fminimum_mag((double)x, (double)y); }
double fmaximum_num(double, double);
long double fmaximum_numl(long double x, long double y) { return (long double)fmaximum_num((double)x, (double)y); }
double fminimum_num(double, double);
long double fminimum_numl(long double x, long double y) { return (long double)fminimum_num((double)x, (double)y); }
double fmaximum_mag_num(double, double);
long double fmaximum_mag_numl(long double x, long double y) { return (long double)fmaximum_mag_num((double)x, (double)y); }
double fminimum_mag_num(double, double);
long double fminimum_mag_numl(long double x, long double y) { return (long double)fminimum_mag_num((double)x, (double)y); }
#endif
#else
typedef int __axc_ldwide_c89;
#endif

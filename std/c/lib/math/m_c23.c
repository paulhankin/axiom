/* m_c23.c - the functions C23 adds to <math.h> (7.12) that work on the
   representation, and the long double forms of those that compute (the
   double and float ones are std/c/rt/math_c23.ax's and math_narrow.ax's):

   - fromfp, ufromfp, fromfpx, ufromfpx (7.12.9.10-11, F.10.6.10-11): the
     integer x rounds to in the direction rnd, if a signed (unsigned) integer
     of width bits holds it; else (and for width 0, NaN and infinities) a
     domain error. fromfpx raises "inexact" when the result is not x; fromfp
     raises nothing more (ceil, floor, ... do not raise "inexact" here). A rnd
     other than the FP_INT_ macros rounds to nearest, ties to even.
   - getpayload, setpayload, setpayloadsig (7.12.13, F.10.13): the payload is
     the NaN's fraction bits below the quiet bit (51 of double, 22 of float,
     62 of the x87's format, 111 of binary128), as nan() writes it; a payload
     argument must be an integer of that range (and nonzero for a signaling
     NaN, whose fraction must not be zero).
   - totalorder, totalordermag (7.12.15, F.10.12): IEEE 754's totalOrder, on
     the encodings: -NaN < -inf < ... < -0 < +0 < ... < +inf < +NaN, the
     NaNs of one sign ordered by their fraction (signaling below quiet).
   - canonicalize (7.12.11.7): every double and float encoding is canonical
     (a signaling NaN becomes quiet, with "invalid"); of the x87's format,
     pseudo-denormals have a canonical form and unnormals, pseudo-infinities
     and pseudo-NaNs are not values (the result is nonzero, nothing stored).
   - iseqsig (7.12.17.7) through __axc_iseqsig*: equality, with "invalid"
     and a domain error for a NaN operand.
   - llogb (7.12.6.10): ilogb's value as a long, FP_LLOGB0 for 0 and
     FP_LLOGBNAN for NaNs and infinities, which are domain errors. */
#if __STDC_VERSION__ >= 199901L
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>

void __axc_math_err(int e, unsigned int flags);
/* (C23's, declared by <math.h> in C23 only) */
double roundeven(double);
long double roundevenl(long double);

#define FE_INVALID_ 0x01
#define FE_INEXACT_ 0x20
#define LDBL_WIDE (__AXIOM_LDBL_WIDTH__ != 64)
#define X87 (__AXIOM_LDBL_WIDTH__ == 80)

typedef union {
    double d;
    unsigned long long u;
} DB;
typedef union {
    float f;
    unsigned int u;
} FB;
typedef union {
    long double v;
    unsigned char b[16];
} LB;

static double domain_d(void)
{
    __axc_math_err(EDOM, FE_INVALID_);
    return (double)NAN;
}

/* -------------------------------------------------------------------------
   fromfp, ufromfp, fromfpx, ufromfpx */

#define FP_INT_UPWARD_ 0
#define FP_INT_DOWNWARD_ 1
#define FP_INT_TOWARDZERO_ 2
#define FP_INT_TONEARESTFROMZERO_ 3
#define FP_INT_TONEAREST_ 4

static double rnd_d(double x, int rnd)
{
    switch (rnd) {
    case FP_INT_UPWARD_: return ceil(x);
    case FP_INT_DOWNWARD_: return floor(x);
    case FP_INT_TOWARDZERO_: return trunc(x);
    case FP_INT_TONEARESTFROMZERO_: return round(x);
    default: return roundeven(x);
    }
}

/* r (an integer) is in the range of a width-bit integer (unsigned if uns) */
static int fits_d(double r, unsigned int width, int uns)
{
    if (uns)
        return !(r < 0.0) && (width >= 1024 || r < ldexp(1.0, (int)width));
    if (width > 1024)
        return 1;
    return r >= -ldexp(1.0, (int)width - 1) && r < ldexp(1.0, (int)width - 1);
}

static double fromfp_d(double x, int rnd, unsigned int width, int uns, int inexact)
{
    double r;
    if (width == 0 || x != x || x - x != 0.0)
        return domain_d();
    r = rnd_d(x, rnd);
    if (!fits_d(r, width, uns))
        return domain_d();
    if (inexact && r != x)
        __axc_math_err(0, FE_INEXACT_);
    return r;
}

double fromfp(double x, int rnd, unsigned int width) { return fromfp_d(x, rnd, width, 0, 0); }
double ufromfp(double x, int rnd, unsigned int width) { return fromfp_d(x, rnd, width, 1, 0); }
double fromfpx(double x, int rnd, unsigned int width) { return fromfp_d(x, rnd, width, 0, 1); }
double ufromfpx(double x, int rnd, unsigned int width) { return fromfp_d(x, rnd, width, 1, 1); }
/* (a float's integer is exact as a double) */
float fromfpf(float x, int rnd, unsigned int width) { return (float)fromfp_d(x, rnd, width, 0, 0); }
float ufromfpf(float x, int rnd, unsigned int width) { return (float)fromfp_d(x, rnd, width, 1, 0); }
float fromfpxf(float x, int rnd, unsigned int width) { return (float)fromfp_d(x, rnd, width, 0, 1); }
float ufromfpxf(float x, int rnd, unsigned int width) { return (float)fromfp_d(x, rnd, width, 1, 1); }

#if LDBL_WIDE
static long double rnd_l(long double x, int rnd)
{
    switch (rnd) {
    case FP_INT_UPWARD_: return ceill(x);
    case FP_INT_DOWNWARD_: return floorl(x);
    case FP_INT_TOWARDZERO_: return truncl(x);
    case FP_INT_TONEARESTFROMZERO_: return roundl(x);
    default: return roundevenl(x);
    }
}

static long double fromfp_l(long double x, int rnd, unsigned int width, int uns, int inexact)
{
    long double r;
    if (width == 0 || x != x || x - x != 0.0L)
        return (long double)domain_d();
    r = rnd_l(x, rnd);
    if (uns) {
        if (r < 0.0L || (width < 16384 && r >= ldexpl(1.0L, (int)width)))
            return (long double)domain_d();
    } else if (width <= 16384 && (r < -ldexpl(1.0L, (int)width - 1) || r >= ldexpl(1.0L, (int)width - 1))) {
        return (long double)domain_d();
    }
    if (inexact && r != x)
        __axc_math_err(0, FE_INEXACT_);
    return r;
}
#else
#define fromfp_l(x, rnd, w, u, i) ((long double)fromfp_d((double)(x), rnd, w, u, i))
#endif
long double fromfpl(long double x, int rnd, unsigned int width) { return fromfp_l(x, rnd, width, 0, 0); }
long double ufromfpl(long double x, int rnd, unsigned int width) { return fromfp_l(x, rnd, width, 1, 0); }
long double fromfpxl(long double x, int rnd, unsigned int width) { return fromfp_l(x, rnd, width, 0, 1); }
long double ufromfpxl(long double x, int rnd, unsigned int width) { return fromfp_l(x, rnd, width, 1, 1); }

/* -------------------------------------------------------------------------
   The long double's bits: (sign, 15-bit exponent field, 128-bit
   significand field: the x87's 64 bits in lo, binary128's 112 in hi:lo) */

typedef struct {
    int neg;
    unsigned e;
    unsigned long long hi, lo;
} LP;

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

#if LDBL_WIDE
static LP lparts(long double x)
{
    LB u;
    LP r;
    unsigned se;
    u.v = x;
    if (X87) {
        se = (unsigned)u.b[8] | ((unsigned)u.b[9] << 8);
        r.hi = 0;
        r.lo = get64(u.b);
    } else {
        unsigned long long h = get64(u.b + 8);
        se = (unsigned)(h >> 48);
        r.hi = h & 0xffffffffffffULL;
        r.lo = get64(u.b);
    }
    r.neg = (int)((se >> 15) & 1);
    r.e = se & 0x7fff;
    return r;
}

static long double lmake(LP p)
{
    LB u;
    unsigned se = ((unsigned)p.neg << 15) | p.e;
    int i;
    for (i = 0; i < 16; i++)
        u.b[i] = 0;
    put64(u.b, p.lo);
    if (X87) {
        u.b[8] = (unsigned char)(se & 0xff);
        u.b[9] = (unsigned char)(se >> 8);
    } else {
        put64(u.b + 8, (p.hi & 0xffffffffffffULL) | ((unsigned long long)se << 48));
    }
    return u.v;
}

/* the quiet bit, as (hi, lo) bits of the significand field */
#define QH (X87 ? 0ULL : 0x800000000000ULL)
#define QL (X87 ? 0x4000000000000000ULL : 0ULL)

static int lnan(LP p)
{
    if (p.e != 0x7fff)
        return 0;
    if (X87)
        return (p.lo << 1) != 0; /* (pseudo-NaNs too) */
    return (p.hi | p.lo) != 0;
}

/* pl as a nonnegative integer below 2^n (n = 62 or 111) in (hi, lo), or 0
   if it is not one */
static int payload_bits(long double pl, int n, unsigned long long *hi, unsigned long long *lo)
{
    LP p;
    int e, sh;
    unsigned long long mh, ml;
    if (!(pl >= 0.0L) || pl != truncl(pl))
        return 0;
    *hi = *lo = 0;
    if (pl == 0.0L)
        return 1;
    if (pl >= ldexpl(1.0L, n))
        return 0;
    p = lparts(pl);
    e = (int)p.e - 16383; /* pl in [2^e, 2^(e+1)) */
    if (X87) {
        mh = 0;
        ml = p.lo; /* integer bit at 63 */
        sh = 63 - e;
    } else {
        mh = p.hi | 0x1000000000000ULL; /* the leading bit at 112 */
        ml = p.lo;
        sh = 112 - e;
    }
    if (sh >= 64) {
        ml = mh >> (sh - 64);
        mh = 0;
    } else if (sh > 0) {
        ml = (ml >> sh) | (mh << (64 - sh));
        mh >>= sh;
    }
    *hi = mh;
    *lo = ml;
    return 1;
}
#endif

/* -------------------------------------------------------------------------
   getpayload, setpayload, setpayloadsig */

double getpayload(const double *x)
{
    DB u;
    u.d = *x;
    if (*x == *x)
        return -1.0;
    return (double)(u.u & 0x7ffffffffffffULL);
}

float getpayloadf(const float *x)
{
    FB u;
    u.f = *x;
    if (*x == *x)
        return -1.0f;
    return (float)(u.u & 0x3fffffu);
}

static int setpl_d(double *res, double pl, int sig)
{
    DB u;
    if (!(pl >= (sig ? 1.0 : 0.0) && pl < 2251799813685248.0) || pl != trunc(pl)) {
        *res = 0.0;
        return 1;
    }
    u.u = (sig ? 0x7ff0000000000000ULL : 0x7ff8000000000000ULL) | (unsigned long long)pl;
    *res = u.d;
    return 0;
}

int setpayload(double *res, double pl) { return setpl_d(res, pl, 0); }
int setpayloadsig(double *res, double pl) { return setpl_d(res, pl, 1); }

static int setpl_f(float *res, float pl, int sig)
{
    FB u;
    if (!(pl >= (sig ? 1.0f : 0.0f) && pl < 4194304.0f) || pl != truncf(pl)) {
        *res = 0.0f;
        return 1;
    }
    u.u = (sig ? 0x7f800000u : 0x7fc00000u) | (unsigned int)pl;
    *res = u.f;
    return 0;
}

int setpayloadf(float *res, float pl) { return setpl_f(res, pl, 0); }
int setpayloadsigf(float *res, float pl) { return setpl_f(res, pl, 1); }

#if LDBL_WIDE
long double getpayloadl(const long double *x)
{
    LP p = lparts(*x);
    unsigned long long h, l;
    if (*x == *x)
        return -1.0L;
    h = p.hi & ~QH & (X87 ? 0 : 0xffffffffffffULL);
    l = p.lo & ~QL & (X87 ? 0x3fffffffffffffffULL : ~0ULL);
    return ldexpl((long double)h, 64) + (long double)l;
}

static int setpl_l(long double *res, long double pl, int sig)
{
    unsigned long long h, l;
    LP p;
    if (!payload_bits(pl, X87 ? 62 : 111, &h, &l) || (sig && (h | l) == 0)) {
        *res = 0.0L;
        return 1;
    }
    p.neg = 0;
    p.e = 0x7fff;
    p.hi = h | (sig ? 0 : QH);
    p.lo = l | (sig ? 0 : QL) | (X87 ? 0x8000000000000000ULL : 0);
    *res = lmake(p);
    return 0;
}

int setpayloadl(long double *res, long double pl) { return setpl_l(res, pl, 0); }
int setpayloadsigl(long double *res, long double pl) { return setpl_l(res, pl, 1); }
#else
long double getpayloadl(const long double *x)
{
    double d = (double)*x;
    return (long double)getpayload(&d);
}

int setpayloadl(long double *res, long double pl)
{
    double r;
    int k = setpayload(&r, (double)pl);
    *res = r;
    return k;
}

int setpayloadsigl(long double *res, long double pl)
{
    double r;
    int k = setpayloadsig(&r, (double)pl);
    *res = r;
    return k;
}
#endif

/* -------------------------------------------------------------------------
   totalorder, totalordermag: the encodings as integers that order as the
   values, negatives reversed */

static unsigned long long key_d(unsigned long long u)
{
    return (u >> 63) ? ~u : (u | 0x8000000000000000ULL);
}

int totalorder(const double *x, const double *y)
{
    DB a, b;
    a.d = *x;
    b.d = *y;
    return key_d(a.u) <= key_d(b.u);
}

int totalordermag(const double *x, const double *y)
{
    DB a, b;
    a.d = *x;
    b.d = *y;
    return (a.u & 0x7fffffffffffffffULL) <= (b.u & 0x7fffffffffffffffULL);
}

static unsigned int key_f(unsigned int u)
{
    return (u >> 31) ? ~u : (u | 0x80000000u);
}

int totalorderf(const float *x, const float *y)
{
    FB a, b;
    a.f = *x;
    b.f = *y;
    return key_f(a.u) <= key_f(b.u);
}

int totalordermagf(const float *x, const float *y)
{
    FB a, b;
    a.f = *x;
    b.f = *y;
    return (a.u & 0x7fffffffu) <= (b.u & 0x7fffffffu);
}

#if LDBL_WIDE
/* |x| <= |y| on the encodings: exponent field, then significand field */
static int le_mag(LP a, LP b)
{
    if (a.e != b.e)
        return a.e < b.e;
    if (a.hi != b.hi)
        return a.hi < b.hi;
    return a.lo <= b.lo;
}

int totalorderl(const long double *x, const long double *y)
{
    LP a = lparts(*x), b = lparts(*y);
    if (a.neg != b.neg)
        return a.neg;
    if (a.neg)
        return le_mag(b, a);
    return le_mag(a, b);
}

int totalordermagl(const long double *x, const long double *y)
{
    return le_mag(lparts(*x), lparts(*y));
}
#else
int totalorderl(const long double *x, const long double *y)
{
    double a = (double)*x, b = (double)*y;
    return totalorder(&a, &b);
}

int totalordermagl(const long double *x, const long double *y)
{
    double a = (double)*x, b = (double)*y;
    return totalordermag(&a, &b);
}
#endif

/* -------------------------------------------------------------------------
   canonicalize */

int canonicalize(double *cx, const double *x)
{
    DB u;
    u.d = *x;
    if ((u.u & 0x7ff8000000000000ULL) == 0x7ff0000000000000ULL && (u.u & 0x7ffffffffffffULL) != 0) {
        __axc_math_err(0, FE_INVALID_); /* a signaling NaN: quieted */
        u.u |= 0x0008000000000000ULL;
    }
    *cx = u.d;
    return 0;
}

int canonicalizef(float *cx, const float *x)
{
    FB u;
    u.f = *x;
    if ((u.u & 0x7fc00000u) == 0x7f800000u && (u.u & 0x3fffffu) != 0) {
        __axc_math_err(0, FE_INVALID_);
        u.u |= 0x00400000u;
    }
    *cx = u.f;
    return 0;
}

#if LDBL_WIDE
int canonicalizel(long double *cx, const long double *x)
{
    LP p = lparts(*x);
    if (X87) {
        int ib = (int)(p.lo >> 63);
        if (p.e == 0 && ib)
            p.e = 1; /* a pseudo-denormal: the same value, normal */
        else if (p.e != 0 && !ib)
            return 1; /* an unnormal, pseudo-infinity or pseudo-NaN */
    }
    if (lnan(p) && ((p.hi & QH) | (p.lo & QL)) == 0) {
        __axc_math_err(0, FE_INVALID_);
        p.hi |= QH;
        p.lo |= QL;
    }
    *cx = lmake(p);
    return 0;
}
#else
int canonicalizel(long double *cx, const long double *x)
{
    double r, d = (double)*x;
    int k = canonicalize(&r, &d);
    *cx = r;
    return k;
}
#endif

/* -------------------------------------------------------------------------
   iseqsig */

int __axc_iseqsig(double x, double y)
{
    if (x != x || y != y) {
        __axc_math_err(EDOM, FE_INVALID_);
        return 0;
    }
    return x == y;
}

int __axc_iseqsigf(float x, float y)
{
    return __axc_iseqsig(x, y);
}

int __axc_iseqsigl(long double x, long double y)
{
    if (x != x || y != y) {
        __axc_math_err(EDOM, FE_INVALID_);
        return 0;
    }
    return x == y;
}

/* -------------------------------------------------------------------------
   llogb */

static long llogb_special(int zero)
{
    __axc_math_err(EDOM, FE_INVALID_);
    return zero ? -LONG_MAX - 1 : LONG_MAX;
}

long llogb(double x)
{
    if (x == 0.0)
        return llogb_special(1);
    if (x != x || x - x != 0.0)
        return llogb_special(0);
    return (long)ilogb(x);
}

long llogbf(float x)
{
    return llogb((double)x);
}

long llogbl(long double x)
{
    if (x == 0.0L)
        return llogb_special(1);
    if (x != x || x - x != 0.0L)
        return llogb_special(0);
    return (long)ilogbl(x);
}

/* -------------------------------------------------------------------------
   The long double forms of the C23 functions that compute */

double exp2m1(double), exp10m1(double), log2p1(double), log10p1(double), logp1(double), rsqrt(double);
double tanpi(double), asinpi(double), acospi(double), atanpi(double), atan2pi(double, double);
double rootn(double, long long), pown(double, long long), powr(double, double), compoundn(double, long long);

#if LDBL_WIDE
/* std/c/rt/math_ld.ax's __axc_ld23, by address, with the format */
#define F (__AXIOM_LDBL_WIDTH__ == 80)
void __axc_ld23(unsigned long long op, unsigned long long f, long double *r, const long double *x,
                const long double *y);
#define LD23(name, op) \
    long double name(long double x) \
    { \
        long double r, y = 0.0L; \
        __axc_ld23(op, F, &r, &x, &y); \
        return r; \
    }
LD23(exp2m1l, 0)
LD23(exp10m1l, 1)
LD23(log2p1l, 2)
LD23(log10p1l, 3)
LD23(rsqrtl, 4)
LD23(sinpil, 5)
LD23(cospil, 6)
LD23(tanpil, 7)
LD23(asinpil, 8)
LD23(acospil, 9)
LD23(atanpil, 10)

long double logp1l(long double x)
{
    return log1pl(x);
}

long double atan2pil(long double y, long double x)
{
    long double r;
    __axc_ld23(11, F, &r, &y, &x);
    return r;
}

long double rootnl(long double x, long long n)
{
    long double r, y = (long double)n;
    __axc_ld23(12, F, &r, &x, &y);
    return r;
}

long double compoundnl(long double x, long long n)
{
    long double r, y = (long double)n;
    __axc_ld23(13, F, &r, &x, &y);
    return r;
}

/* (every long long is exact in the format: powl's integer y) */
long double pownl(long double x, long long n)
{
    if (n == 0)
        return 1.0L;
    return powl(x, (long double)n);
}

/* powr's special cases (F.10.4.7), then powl's value */
long double powrl(long double x, long double y)
{
    if (x != x || y != y)
        return x + y;
    if (x < 0.0L || (x == 0.0L && y == 0.0L) || (x - x != 0.0L && y == 0.0L) || (x == 1.0L && y - y != 0.0L))
        return (long double)domain_d();
    if (x == 0.0L) {
        if (y < 0.0L)
            return y - y == 0.0L ? (long double)powr(0.0, -1.0) : (long double)INFINITY; /* a pole for finite y */
        return 0.0L;
    }
    if (x == 1.0L || y == 0.0L)
        return 1.0L;
    return powl(x, y);
}
#else
long double exp2m1l(long double x) { return (long double)exp2m1((double)x); }
long double exp10m1l(long double x) { return (long double)exp10m1((double)x); }
long double log2p1l(long double x) { return (long double)log2p1((double)x); }
long double log10p1l(long double x) { return (long double)log10p1((double)x); }
long double logp1l(long double x) { return (long double)logp1((double)x); }
long double rsqrtl(long double x) { return (long double)rsqrt((double)x); }
double sinpi(double), cospi(double);
long double sinpil(long double x) { return (long double)sinpi((double)x); }
long double cospil(long double x) { return (long double)cospi((double)x); }
long double tanpil(long double x) { return (long double)tanpi((double)x); }
long double asinpil(long double x) { return (long double)asinpi((double)x); }
long double acospil(long double x) { return (long double)acospi((double)x); }
long double atanpil(long double x) { return (long double)atanpi((double)x); }
long double atan2pil(long double y, long double x) { return (long double)atan2pi((double)y, (double)x); }
long double rootnl(long double x, long long n) { return (long double)rootn((double)x, n); }
long double pownl(long double x, long long n) { return (long double)pown((double)x, n); }
long double powrl(long double x, long double y) { return (long double)powr((double)x, (double)y); }
long double compoundnl(long double x, long long n) { return (long double)compoundn((double)x, n); }
#endif

#else
typedef int __axc_c23_c89;
#endif

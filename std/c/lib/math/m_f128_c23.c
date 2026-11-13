/* m_f128_c23.c - the _Float128 forms of the functions C23 adds to <math.h>
   (C23 Annex H.11: exp2m1f128 ... compoundnf128, fromfpf128 and its kin,
   llogbf128, getpayloadf128, setpayloadf128, setpayloadsigf128,
   totalorderf128, totalordermagf128, canonicalizef128), binary128 on every
   target whatever long double is: those that work on the representation as
   m_c23.c's long double ones do for a binary128 long double (the same rules:
   the payload is the 111 fraction bits below the quiet bit; total order on
   the encodings), those that compute std/c/rt/math_ld.ax's kernels in
   binary128 (format 0, as a binary128 long double's). */
#if __STDC_VERSION__ >= 199901L
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <errno.h>
#include <limits.h>
#include <math.h>

void __axc_math_err(int e, unsigned int flags);
void __axc_ld23(unsigned long long op, unsigned long long f, _Float128 *r, const _Float128 *x, const _Float128 *y);

#define FE_INVALID_ 0x01
#define FE_INEXACT_ 0x20
#define Q(c) c##F128

typedef union {
    _Float128 v;
    unsigned char b[16];
} QB;

static _Float128 domain_q(void)
{
    __axc_math_err(EDOM, FE_INVALID_);
    return (_Float128)NAN;
}

/* -------------------------------------------------------------------------
   fromfp, ufromfp, fromfpx, ufromfpx (7.12.9.10-11) */

static _Float128 rnd_q(_Float128 x, int rnd)
{
    switch (rnd) {
    case 0: return ceilf128(x);
    case 1: return floorf128(x);
    case 2: return truncf128(x);
    case 3: return roundf128(x);
    default: return roundevenf128(x);
    }
}

static _Float128 fromfp_q(_Float128 x, int rnd, unsigned int width, int uns, int inexact)
{
    _Float128 r;
    if (width == 0 || x != x || x - x != Q(0.0))
        return domain_q();
    r = rnd_q(x, rnd);
    if (uns) {
        if (r < Q(0.0) || (width < 16384 && r >= ldexpf128(Q(1.0), (int)width)))
            return domain_q();
    } else if (width <= 16384 && (r < -ldexpf128(Q(1.0), (int)width - 1) || r >= ldexpf128(Q(1.0), (int)width - 1))) {
        return domain_q();
    }
    if (inexact && r != x)
        __axc_math_err(0, FE_INEXACT_);
    return r;
}

_Float128 fromfpf128(_Float128 x, int rnd, unsigned int width) { return fromfp_q(x, rnd, width, 0, 0); }
_Float128 ufromfpf128(_Float128 x, int rnd, unsigned int width) { return fromfp_q(x, rnd, width, 1, 0); }
_Float128 fromfpxf128(_Float128 x, int rnd, unsigned int width) { return fromfp_q(x, rnd, width, 0, 1); }
_Float128 ufromfpxf128(_Float128 x, int rnd, unsigned int width) { return fromfp_q(x, rnd, width, 1, 1); }

/* -------------------------------------------------------------------------
   The bits: sign, 15-bit exponent field, 112-bit fraction (hi: 48 bits) */

typedef struct {
    int neg;
    unsigned e;
    unsigned long long hi, lo;
} QP;

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

static QP qparts(_Float128 x)
{
    QB u;
    QP r;
    unsigned long long h;
    u.v = x;
    h = get64(u.b + 8);
    r.neg = (int)(h >> 63);
    r.e = (unsigned)(h >> 48) & 0x7fff;
    r.hi = h & 0xffffffffffffULL;
    r.lo = get64(u.b);
    return r;
}

static _Float128 qmake(QP p)
{
    QB u;
    unsigned long long se = ((unsigned long long)p.neg << 15) | p.e;
    put64(u.b, p.lo);
    put64(u.b + 8, (p.hi & 0xffffffffffffULL) | (se << 48));
    return u.v;
}

#define QH 0x800000000000ULL /* the quiet bit, in hi */

/* pl as a nonnegative integer below 2^111 in (hi, lo), or 0 if it is not one */
static int payload_bits(_Float128 pl, unsigned long long *hi, unsigned long long *lo)
{
    QP p;
    int e, sh;
    unsigned long long mh, ml;
    if (!(pl >= Q(0.0)) || pl != truncf128(pl))
        return 0;
    *hi = *lo = 0;
    if (pl == Q(0.0))
        return 1;
    if (pl >= ldexpf128(Q(1.0), 111))
        return 0;
    p = qparts(pl);
    e = (int)p.e - 16383; /* pl in [2^e, 2^(e+1)) */
    mh = p.hi | 0x1000000000000ULL;
    ml = p.lo;
    sh = 112 - e;
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

/* -------------------------------------------------------------------------
   getpayload, setpayload, setpayloadsig (7.12.13, F.10.13) */

_Float128 getpayloadf128(const _Float128 *x)
{
    QP p = qparts(*x);
    if (*x == *x)
        return Q(-1.0);
    return ldexpf128((_Float128)(p.hi & ~QH), 64) + (_Float128)p.lo;
}

static int setpl_q(_Float128 *res, _Float128 pl, int sig)
{
    unsigned long long h, l;
    QP p;
    if (!payload_bits(pl, &h, &l) || (sig && (h | l) == 0)) {
        *res = Q(0.0);
        return 1;
    }
    p.neg = 0;
    p.e = 0x7fff;
    p.hi = h | (sig ? 0 : QH);
    p.lo = l;
    *res = qmake(p);
    return 0;
}

int setpayloadf128(_Float128 *res, _Float128 pl) { return setpl_q(res, pl, 0); }
int setpayloadsigf128(_Float128 *res, _Float128 pl) { return setpl_q(res, pl, 1); }

/* -------------------------------------------------------------------------
   totalorder, totalordermag (7.12.15, F.10.12): on the encodings */

static int le_mag(QP a, QP b)
{
    if (a.e != b.e)
        return a.e < b.e;
    if (a.hi != b.hi)
        return a.hi < b.hi;
    return a.lo <= b.lo;
}

int totalorderf128(const _Float128 *x, const _Float128 *y)
{
    QP a = qparts(*x), b = qparts(*y);
    if (a.neg != b.neg)
        return a.neg;
    if (a.neg)
        return le_mag(b, a);
    return le_mag(a, b);
}

int totalordermagf128(const _Float128 *x, const _Float128 *y)
{
    return le_mag(qparts(*x), qparts(*y));
}

/* canonicalize (7.12.11.7): every encoding is canonical; a signaling NaN
   becomes quiet, with "invalid" */
int canonicalizef128(_Float128 *cx, const _Float128 *x)
{
    QP p = qparts(*x);
    if (p.e == 0x7fff && (p.hi | p.lo) != 0 && (p.hi & QH) == 0) {
        __axc_math_err(0, FE_INVALID_);
        p.hi |= QH;
    }
    *cx = qmake(p);
    return 0;
}

/* llogb (7.12.6.10) */
long llogbf128(_Float128 x)
{
    if (x == Q(0.0) || x != x || x - x != Q(0.0)) {
        __axc_math_err(EDOM, FE_INVALID_);
        return x == Q(0.0) ? -LONG_MAX - 1 : LONG_MAX;
    }
    return (long)ilogbf128(x);
}

/* -------------------------------------------------------------------------
   The functions that compute: math_ld.ax's __axc_ld23 in binary128 */

#define Q23(name, op) \
    _Float128 name(_Float128 x) \
    { \
        _Float128 r, y = Q(0.0); \
        __axc_ld23(op, 0, &r, &x, &y); \
        return r; \
    }
Q23(exp2m1f128, 0)
Q23(exp10m1f128, 1)
Q23(log2p1f128, 2)
Q23(log10p1f128, 3)
Q23(rsqrtf128, 4)
Q23(sinpif128, 5)
Q23(cospif128, 6)
Q23(tanpif128, 7)
Q23(asinpif128, 8)
Q23(acospif128, 9)
Q23(atanpif128, 10)

_Float128 logp1f128(_Float128 x)
{
    return log1pf128(x);
}

_Float128 atan2pif128(_Float128 y, _Float128 x)
{
    _Float128 r;
    __axc_ld23(11, 0, &r, &y, &x);
    return r;
}

_Float128 rootnf128(_Float128 x, long long n)
{
    _Float128 r, y = (_Float128)n;
    __axc_ld23(12, 0, &r, &x, &y);
    return r;
}

_Float128 compoundnf128(_Float128 x, long long n)
{
    _Float128 r, y = (_Float128)n;
    __axc_ld23(13, 0, &r, &x, &y);
    return r;
}

/* (every long long is exact in binary128: powf128's integer y) */
_Float128 pownf128(_Float128 x, long long n)
{
    if (n == 0)
        return Q(1.0);
    return powf128(x, (_Float128)n);
}

double powr(double, double);

/* powr's special cases (F.10.4.7), then powf128's value */
_Float128 powrf128(_Float128 x, _Float128 y)
{
    if (x != x || y != y)
        return x + y;
    if (x < Q(0.0) || (x == Q(0.0) && y == Q(0.0)) || (x - x != Q(0.0) && y == Q(0.0)) ||
        (x == Q(1.0) && y - y != Q(0.0)))
        return domain_q();
    if (x == Q(0.0)) {
        if (y < Q(0.0))
            return y - y == Q(0.0) ? (_Float128)powr(0.0, -1.0) : (_Float128)INFINITY; /* a pole for finite y */
        return Q(0.0);
    }
    if (x == Q(1.0) || y == Q(0.0))
        return Q(1.0);
    return powf128(x, y);
}
#else
typedef int __axc_f128_c23_c89;
#endif

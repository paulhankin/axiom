/* complex_impl.h - the functions of <complex.h> (C17 7.3.5-7.3.9), written
   once for double (complex.c), long double (complexl.c) and _Float128
   (complexf128.c), with the special values of Annex G (G.6) and the methods
   of W. Kahan, "Branch cuts for complex elementary functions, or much ado
   about nothing's sign bit" (1987): csqrt without overflow, casin and cacos
   from the square roots of 1 - z and 1 + z, catanh with log1p.

   Each part of a result is computed in pairs of the real type (h + l, about
   twice its precision: Dekker's and Knuth's error-free transformations, T. J.
   Dekker, "A floating-point technique for extending the available
   precision", Numer. Math. 18, 1971) from real functions that give their
   values in such pairs (the including unit's XK1 and XK2: within about
   2^-(p + 30) for a type of p bits), and rounded once, so that each part is
   within about half an ulp and a small fraction (docs/c.md gives the
   measured errors). The formulas are those in which nothing cancels:
   csinh and ccosh from sinh x, cosh x, sin y and cos y; ctanh from
   sinh x cosh x / (sinh^2 x + cos^2 y) and sin y cos y / (sinh^2 x +
   cos^2 y); catanh's real part from log1p of 4|x| / ((1 - |x|)^2 + y^2),
   near +-1 from the difference of two logarithms; e^x beyond the overflow
   of its products as (e^(x/2))^2.

   The including unit defines T (the real type), CT (the complex type), C,
   RE, IM, INF, FN(n) (this file's functions' names: cexp, or cexpl), M(n)
   (the real functions': exp, or expl), PROD_ERR(a, b, p) (a b - p exactly,
   for p = a b rounded and finite), XK1(op, x, r) (r[0] + r[1], and r[2] +
   r[3]: op 0 e^x for finite x, inf beyond the overflow; 1 e^x - 1 for |x| <=
   1; 2 sin x and cos x, finite x; 3 log1p x, x >= 0), XK2(op, y, x, r) (op 0
   atan2(y, x), finite, not both 0; 1 log|y + ix| for y >= x >= 0, y > 0
   finite), and the constants: K_EXP_BIG (below the overflow threshold of
   e^x), K_SQ_HI and K_SQ_LO (the binary exponents between which squares and
   their pairs neither overflow nor lose their low parts; K_SQ_HIV and
   K_SQ_LOV the powers of two), K_TANH_BIG (where
   tanh x is 1 to the type's precision with 2^-10 of an ulp to spare),
   K_ASIN_BIG (beyond which casin is log(2z)'s), K_ATANH_BIG (beyond which
   catanh's real part is Re(1/z)'s), and pi/4, pi/2, 3pi/4, pi and ln 2
   (K_PIO4 ... K_LN2, and K_LN2_LO: ln 2 - K_LN2). */

/* Pairs of T: h + l, |l| <= ulp(h)/2 */
typedef struct {
    T h, l;
} DT;

static DT dt(T h, T l)
{
    DT r;
    r.h = h;
    r.l = l;
    return r;
}

static DT dt_sum(T a, T b)
{
    T s = a + b, bb;
    if (!isfinite(s))
        return dt(s, 0.0);
    bb = s - a;
    return dt(s, (a - (s - bb)) + (b - bb));
}

static DT dt_prod(T a, T b)
{
    T p = a * b;
    if (!isfinite(p) || p == 0.0)
        return dt(p, 0.0);
    return dt(p, PROD_ERR(a, b, p));
}

static DT dt_add(DT a, DT b)
{
    DT s = dt_sum(a.h, b.h);
    if (!isfinite(s.h))
        return s;
    return dt_sum(s.h, s.l + (a.l + b.l));
}

static DT dt_neg(DT a)
{
    return dt(-a.h, -a.l);
}

static DT dt_sub(DT a, DT b)
{
    return dt_add(a, dt_neg(b));
}

static DT dt_mul(DT a, DT b)
{
    DT p = dt_prod(a.h, b.h);
    if (!isfinite(p.h))
        return p;
    return dt_sum(p.h, p.l + (a.h * b.l + a.l * b.h));
}

static DT dt_div(DT a, DT b)
{
    T q = a.h / b.h;
    DT r;
    if (!isfinite(q) || q == 0.0)
        return dt(q, 0.0);
    r = dt_add(a, dt_mul(b, dt(-q, 0.0)));
    return dt_sum(q, (r.h + r.l) / b.h);
}

/* a 2^k (exact while nothing underflows) */
static DT dt_scale(DT a, int k)
{
    return dt(M(scalbn)(a.h, k), M(scalbn)(a.l, k));
}

static DT dt_half(DT a)
{
    return dt(0.5 * a.h, 0.5 * a.l);
}

/* sqrt(a), a >= 0 (one Newton step from the square root of the head) */
static DT dt_sqrt(DT a)
{
    T s;
    DT e;
    if (a.h <= 0.0)
        return dt(0.0, 0.0);
    s = M(sqrt)(a.h);
    e = dt_add(a, dt_prod(-s, s));
    return dt_sum(s, (e.h + e.l) / (2.0 * s));
}

/* the pair rounded (the head's sign kept for a zero) */
static T dt_round(DT a)
{
    if (a.l == 0.0)
        return a.h;
    return a.h + a.l;
}

/* The real functions in pairs, of pairs: the head's value with the first
   term of the tail's (the tail is below 2^-p of the head) */
static DT xk1(int op, T x)
{
    T r[4];
    XK1(op, x, r);
    return dt(r[0], r[1]);
}

static DT xexp(DT x)
{
    DT e = xk1(0, x.h);
    if (x.l != 0.0 && isfinite(e.h))
        e = dt_add(e, dt(e.h * x.l, 0.0));
    return e;
}

static void xsincos(DT y, DT *s, DT *c)
{
    T r[4];
    XK1(2, y.h, r);
    *s = dt(r[0], r[1]);
    *c = dt(r[2], r[3]);
    if (y.l != 0.0) {
        DT s0 = *s, c0 = *c;
        if (M(fabs)(y.l) < 0x1p-30) {
            *s = dt_add(s0, dt(c0.h * y.l, 0.0));
            *c = dt_add(c0, dt(-(s0.h * y.l), 0.0));
        } else {
            /* (a tail beyond 2^-30: the sum's sine and cosine) */
            T q[4];
            DT sl, cl;
            XK1(2, y.l, q);
            sl = dt(q[0], q[1]);
            cl = dt(q[2], q[3]);
            *s = dt_add(dt_mul(s0, cl), dt_mul(c0, sl));
            *c = dt_sub(dt_mul(c0, cl), dt_mul(s0, sl));
        }
    }
}

/* log1p x, x >= 0 */
static DT xlog1p(DT x)
{
    DT l = xk1(3, x.h);
    if (x.l != 0.0)
        l = dt_add(l, dt(x.l / (1.0 + x.h), 0.0));
    return l;
}

static DT xk2(int op, T y, T x)
{
    T r[4];
    XK2(op, y, x, r);
    return dt(r[0], r[1]);
}

/* log|a + ib|, a >= 0, b >= 0, not both 0 */
static DT xhalflog(T a, T b)
{
    if (a >= b)
        return xk2(1, a, b);
    return xk2(1, b, a);
}

/* log v, v > 0 */
static DT xlog(DT v)
{
    DT l = xk2(1, v.h, 0.0);
    if (v.l != 0.0)
        l = dt_add(l, dt(v.l / v.h, 0.0));
    return l;
}

/* atan2(y, x), finite, not both 0: the heads' with the first-order term of
   the tails ((x yl - y xl) / (x^2 + y^2), scaled) */
static DT xatan2(DT y, DT x)
{
    DT a = xk2(0, y.h, x.h);
    if (y.l != 0.0 || x.l != 0.0) {
        T s = M(fabs)(x.h) > M(fabs)(y.h) ? M(fabs)(x.h) : M(fabs)(y.h);
        T xn = x.h, yn = y.h, xl = x.l, yl = y.l;
        if (!(s < K_SQ_HIV && s > K_SQ_LOV)) {
            int e = M(ilogb)(s);
            xn = M(scalbn)(x.h, -e);
            yn = M(scalbn)(y.h, -e);
            xl = M(scalbn)(x.l, -e);
            yl = M(scalbn)(y.l, -e);
        }
        a = dt_add(a, dt((xn * yl - yn * xl) / (xn * xn + yn * yn), 0.0));
    }
    return a;
}

/* asinh b, b >= 0 (below 2^K_SQ_HI): log1p(b + b^2 / (1 + sqrt(1 + b^2))) */
static DT xasinh(DT b)
{
    DT q = dt_mul(b, b);
    DT sq = dt_sqrt(dt_add(q, dt(1.0, 0.0)));
    return xlog1p(dt_add(b, dt_div(q, dt_add(sq, dt(1.0, 0.0)))));
}

/* sinh |x| and cosh |x| for |x| <= K_EXP_BIG: from u = e^|x| - 1 below 1/2
   ((u + u/(u + 1))/2 and 1 + u^2/(2(u + 1))), else from e^|x| and its
   reciprocal */
static void xsinhcosh(T x, DT *sh, DT *ch)
{
    T a = M(fabs)(x);
    if (a < 0.5) {
        DT u = xk1(1, a), u1 = dt_add(u, dt(1.0, 0.0));
        *sh = dt_half(dt_add(u, dt_div(u, u1)));
        *ch = dt_add(dt(1.0, 0.0), dt_half(dt_div(dt_mul(u, u), u1)));
    } else {
        DT e = xk1(0, a), ei = dt_div(dt(1.0, 0.0), e);
        *sh = dt_half(dt_sub(e, ei));
        *ch = dt_half(dt_add(e, ei));
    }
}

/* sqrt(a + ib) for pairs a and b >= 0, not both 0: the real part in *re, the
   imaginary in *im (both >= 0). Kahan's: t = sqrt((|a| + |a + ib|) / 2),
   and b / 2t, scaled by an even power of two into the range of the squares */
static void cs_dt(DT a, DT b, DT *re, DT *im)
{
    T mx = M(fabs)(a.h) > b.h ? M(fabs)(a.h) : b.h;
    int k = 0;
    DT as = a, bs = b, h, t, v;
    if (!(mx < K_SQ_HIV && mx > K_SQ_LOV)) {
        int e = M(ilogb)(mx);
        k = (e % 2 == 0) ? e : e - 1;
        as = dt_scale(a, -k);
        bs = dt_scale(b, -k);
    }
    if (as.h < 0.0)
        as = dt_neg(as);
    h = dt_sqrt(dt_add(dt_mul(as, as), dt_mul(bs, bs)));
    t = dt_sqrt(dt_half(dt_add(as, h)));
    if (k == 0) {
        v = dt_div(b, dt(2.0 * t.h, 2.0 * t.l));
    } else {
        /* b / 2t_true = b / 2t 2^(-k/2): for k < 0 from b scaled (exact) */
        if (k < 0)
            v = dt_scale(dt_div(bs, dt(2.0 * t.h, 2.0 * t.l)), k / 2);
        else
            v = dt_scale(dt_div(b, dt(2.0 * t.h, 2.0 * t.l)), -k / 2);
        t = dt_scale(t, k / 2);
    }
    if (a.h >= 0.0) {
        *re = t;
        *im = v;
    } else {
        *re = v;
        *im = t;
    }
}

/* a quiet NaN made by an invalid operation (raising "invalid") */
static T invalid(void)
{
    volatile T z = 0.0;
    return z / z;
}

T (FN(creal))(CT z)
{
    return RE(z);
}

T (FN(cimag))(CT z)
{
    return IM(z);
}

CT (FN(conj))(CT z)
{
    return C(RE(z), -IM(z));
}

T FN(cabs)(CT z)
{
    return M(hypot)(RE(z), IM(z));
}

T FN(carg)(CT z)
{
    return M(atan2)(IM(z), RE(z));
}

CT FN(cproj)(CT z)
{
    if (isinf(RE(z)) || isinf(IM(z)))
        return C(INF, M(copysign)(0.0, IM(z)));
    return z;
}

/* e^z for a finite x + iy (x and y pairs): e^x cos y + i e^x sin y */
static CT cexp_dt(DT x, DT y)
{
    DT s, c, e;
    xsincos(y, &s, &c);
    if (M(fabs)(x.h) <= K_EXP_BIG) {
        e = xexp(x);
        return C(dt_round(dt_mul(e, c)), dt_round(dt_mul(e, s)));
    }
    /* e^x overflows (or underflows) before its products might not */
    e = xexp(dt_half(x));
    return C(dt_round(dt_mul(dt_mul(e, c), e)), dt_round(dt_mul(dt_mul(e, s), e)));
}

/* e^z (G.6.3.1) */
CT FN(cexp)(CT z)
{
    T x = RE(z), y = IM(z);
    if (y == 0.0 && !isnan(x))
        return C(M(exp)(x), y);
    if (!isfinite(x) || !isfinite(y)) {
        if (isinf(x)) {
            if (x < 0.0) {
                if (!isfinite(y))
                    return C(0.0, M(copysign)(0.0, y));
                return C(0.0 * M(cos)(y), 0.0 * M(sin)(y));
            }
            if (!isfinite(y))
                return C(x, isnan(y) ? y : invalid());
            return C(x * M(cos)(y), x * M(sin)(y));
        }
        if (isnan(x))
            return C(x, x + y);
        return C(invalid() + y, invalid() + y); /* x finite, y inf or NaN */
    }
    return cexp_dt(dt(x, 0.0), dt(y, 0.0));
}

/* log z (G.6.3.2): log|z| + i arg z */
CT FN(clog)(CT z)
{
    T x = RE(z), y = IM(z), ax = M(fabs)(x), ay = M(fabs)(y);
    if (isnan(x) || isnan(y)) {
        if (isinf(x) || isinf(y))
            return C(INF, x + y);
        return C(x + y, x + y);
    }
    if (isinf(x) || isinf(y))
        return C(INF, M(atan2)(y, x));
    if (ax == 0.0 && ay == 0.0) {
        volatile T zero = 0.0;
        return C(-1.0 / zero, M(atan2)(y, x)); /* -inf, "divide-by-zero" */
    }
    return C(dt_round(xhalflog(ax, ay)), M(atan2)(y, x));
}

/* the principal square root (G.6.4.2) */
CT FN(csqrt)(CT z)
{
    T x = RE(z), y = IM(z);
    DT re, im;
    if (isinf(y))
        return C(INF, y);
    if (isnan(x)) {
        return C(x, isnan(y) ? y : x + y);
    }
    if (isinf(x)) {
        if (x > 0.0)
            return C(x, isnan(y) ? y : M(copysign)(0.0, y));
        return C(isnan(y) ? M(fabs)(y - y) : 0.0, isnan(y) ? INF : M(copysign)(INF, y));
    }
    if (isnan(y))
        return C(y + x, y + x);
    if (x == 0.0 && y == 0.0)
        return C(0.0, y);
    cs_dt(dt(x, 0.0), dt(M(fabs)(y), 0.0), &re, &im);
    return C(dt_round(re), M(copysign)(dt_round(im), y));
}

/* z^w = e^(w log z) (G.6.4.1 leaves the special cases to cexp and clog):
   log z and its product with w in pairs, so that the error is about
   |w log z| 2^-2p, not |w log z| 2^-p; where |w| |log z| is beyond
   K_POW_WIDE (whose real functions' pairs are within 2^-(p + 30) or so,
   which the angle Im(w log z) then needs more than), the including unit's
   XPOW(z, w, &r) if it defines one: w log z in triple-double
   (std/c/rt/math_ld.ax's __axc_ldcpow), nonzero where it gave r */
CT FN(cpow)(CT z, CT w)
{
    CT l;
    T x = RE(z), y = IM(z), u = RE(w), v = IM(w);
    if (u == 0.0 && v == 0.0)
        return C(1.0, 0.0);
    if (x == 0.0 && y == 0.0 && v == 0.0 && u > 0.0)
        return C(0.0, 0.0);
    if (isfinite(x) && isfinite(y) && isfinite(u) && isfinite(v) && (x != 0.0 || y != 0.0)) {
        DT lr = xhalflog(M(fabs)(x), M(fabs)(y)), li = xk2(0, y, x);
        DT pr, pi;
#ifdef XPOW
        if ((M(fabs)(u) + M(fabs)(v)) * (M(fabs)(lr.h) + M(fabs)(li.h)) > K_POW_WIDE) {
            /* the terms of w log z beyond what the pairs hold to the
               precision the angle needs: the wider computation */
            CT r;
            if (XPOW(z, w, &r))
                return r;
        }
#endif
        /* (a real w of any size: its products with log z are pairs, as
           exact as the small argument's branch below needs) */
        if (!((M(fabs)(u) < 0x1p60 && M(fabs)(v) < 0x1p60) || v == 0.0))
            goto plain;
        pr = dt_sub(dt_mul(dt(u, 0.0), lr), dt_mul(dt(v, 0.0), li));
        pi = dt_add(dt_mul(dt(u, 0.0), li), dt_mul(dt(v, 0.0), lr));
        if (v == 0.0 && x > 0.0 && y != 0.0 && M(fabs)(pi.h) < K_SQ_LOV * K_SQ_LOV &&
            M(fabs)(pr.h) <= K_EXP_BIG) {
            /* a real w and arg z = y/x (nearly) so small that the angle u arg
               z loses its low parts, or underflows: the angle scaled by 2^s,
               its sine itself, its cosine 1 */
            int s = M(ilogb)(x) - M(ilogb)(y) - 1;
            DT q = dt_div(dt(M(scalbn)(y, s), 0.0), dt(x, 0.0));
            DT ps = dt_mul(dt(u, 0.0), q);
            DT e = xexp(pr);
            return C(dt_round(e), M(scalbn)(dt_round(dt_mul(e, ps)), -s));
        }
        if (M(fabs)(pr.h) < 2.0 * K_EXP_BIG && isfinite(pi.h))
            return cexp_dt(pr, pi);
    }
plain:
    l = FN(clog)(z);
    return FN(cexp)(C(u * RE(l) - v * IM(l), u * IM(l) + v * RE(l)));
}

/* sinh z (G.6.2.5) */
CT FN(csinh)(CT z)
{
    T x = RE(z), y = IM(z);
    DT s, c, sh, ch;
    if (!isfinite(x) || !isfinite(y)) {
        if (isinf(x) && isfinite(y)) {
            if (y == 0.0)
                return C(x, y);
            return C(x * M(cos)(y), INF * M(sin)(y));
        }
        if (x == 0.0)
            return C(x, (y - y) * 0.0 + (isnan(y) ? y : invalid()));
        if (isinf(x))
            return C(x, isnan(y) ? y : invalid());
        if (y == 0.0)
            return C(x, y);
        return C(x + y + (isinf(y) ? invalid() : 0.0), x + y);
    }
    if (y == 0.0)
        return C(M(sinh)(x), y);
    xsincos(dt(y, 0.0), &s, &c);
    if (M(fabs)(x) > K_EXP_BIG) {
        /* e^|x| / 2 overflows before its products might not: e^(|x|/2) twice */
        DT e = xk1(0, 0.5 * M(fabs)(x)), he = dt_half(e);
        T re = dt_round(dt_mul(dt_mul(he, c), e));
        return C(x < 0.0 ? -re : re, dt_round(dt_mul(dt_mul(he, s), e)));
    }
    xsinhcosh(x, &sh, &ch);
    if (signbit(x))
        sh = dt_neg(sh);
    return C(dt_round(dt_mul(sh, c)), dt_round(dt_mul(ch, s)));
}

/* cosh z (G.6.2.4) */
CT FN(ccosh)(CT z)
{
    T x = RE(z), y = IM(z);
    DT s, c, sh, ch;
    if (!isfinite(x) || !isfinite(y)) {
        if (isinf(x) && isfinite(y)) {
            if (y == 0.0)
                return C(INF, M(copysign)(0.0, x) * y);
            return C(INF * M(cos)(y), x * M(sin)(y));
        }
        if (x == 0.0)
            return C(isnan(y) ? y : invalid(), 0.0);
        if (isinf(x))
            return C(INF, isnan(y) ? y : invalid());
        if (y == 0.0)
            return C(x, M(copysign)(0.0, y));
        return C(x + y + (isinf(y) ? invalid() : 0.0), x + y);
    }
    if (y == 0.0)
        return C(M(cosh)(x), M(copysign)(0.0, x) * y);
    xsincos(dt(y, 0.0), &s, &c);
    if (M(fabs)(x) > K_EXP_BIG) {
        DT e = xk1(0, 0.5 * M(fabs)(x)), he = dt_half(e);
        T im = dt_round(dt_mul(dt_mul(he, s), e));
        return C(dt_round(dt_mul(dt_mul(he, c), e)), x < 0.0 ? -im : im);
    }
    xsinhcosh(x, &sh, &ch);
    if (signbit(x))
        sh = dt_neg(sh);
    return C(dt_round(dt_mul(ch, c)), dt_round(dt_mul(sh, s)));
}

/* tanh z (G.6.2.6): sinh x cosh x / d + i sin y cos y / d, d = sinh^2 x +
   cos^2 y (= (cosh 2x + cos 2y) / 2: no cancellation) */
CT FN(ctanh)(CT z)
{
    T x = RE(z), y = IM(z);
    DT s, c, sh, ch, d;
    if (!isfinite(x) || !isfinite(y)) {
        if (isinf(x)) {
            if (!isfinite(y))
                return C(M(copysign)(1.0, x), M(copysign)(0.0, y));
            return C(M(copysign)(1.0, x), M(copysign)(0.0, M(sin)(2.0 * y)));
        }
        if (isnan(x)) {
            if (y == 0.0)
                return C(x, y);
            return C(x, x + y);
        }
        if (x == 0.0)
            return C(x, isnan(y) ? y : invalid());
        return C(y - y + (isnan(y) ? y : invalid()), y - y + (isnan(y) ? y : invalid()));
    }
    if (y == 0.0)
        return C(M(tanh)(x), y);
    xsincos(dt(y, 0.0), &s, &c);
    if (M(fabs)(x) > K_TANH_BIG) {
        /* tanh x is 1 to T's precision; the imaginary part 4 sin y cos y
           e^-2|x| (e^-|x| twice: the product may underflow) */
        DT e = xk1(0, -M(fabs)(x));
        {
            DT sc = dt_mul(s, c);
            return C(M(copysign)(1.0, x), dt_round(dt_mul(dt_mul(dt(4.0 * sc.h, 4.0 * sc.l), e), e)));
        }
    }
    xsinhcosh(x, &sh, &ch);
    if (signbit(x))
        sh = dt_neg(sh);
    d = dt_add(dt_mul(sh, sh), dt_mul(c, c));
    return C(dt_round(dt_div(dt_mul(sh, ch), d)), dt_round(dt_div(dt_mul(s, c), d)));
}

/* sin z = -i sinh(iz), cos z = cosh(iz), tan z = -i tanh(iz) (G.6.5) */
CT FN(csin)(CT z)
{
    CT w = FN(csinh)(C(-IM(z), RE(z)));
    return C(IM(w), -RE(w));
}

CT FN(ccos)(CT z)
{
    return FN(ccosh)(C(-IM(z), RE(z)));
}

CT FN(ctan)(CT z)
{
    CT w = FN(ctanh)(C(-IM(z), RE(z)));
    return C(IM(w), -RE(w));
}

/* Kahan's arccosine and arcsine: s1 = sqrt(1 - z), s2 = sqrt(1 + z),
   acos z = 2 atan(re s1 / re s2) + i asinh(im(conj(s2) s1)),
   asin z = atan(re z / re(s1 s2)) + i asinh(im(conj(s1) s2)), whose
   products' terms have one sign. For |z| large (where 1 +- z would lose z's
   low bits, or s1 s2 overflow) they are log(2z)-based. */
static void s12(T x, T y, DT *r1, DT *i1, DT *r2, DT *i2)
{
    T ay = M(fabs)(y);
    cs_dt(dt_sum(1.0, -x), dt(ay, 0.0), r1, i1);
    cs_dt(dt_sum(1.0, x), dt(ay, 0.0), r2, i2);
    /* (the imaginary parts have the signs of -y and y) */
    if (!signbit(y))
        *i1 = dt_neg(*i1);
    else
        *i2 = dt_neg(*i2);
}

/* log 2|z| for the large arguments, as a pair */
static DT log2z(T x, T y)
{
    return dt_add(xhalflog(M(fabs)(x), M(fabs)(y)), dt(K_LN2, K_LN2_LO));
}

CT FN(casin)(CT z)
{
    T x = RE(z), y = IM(z);
    DT r1, i1, r2, i2, a, b;
    T im;
    if (isnan(x) || isnan(y)) {
        if (isinf(x) || isinf(y))
            return C(x + y, M(copysign)(INF, y));
        if (x == 0.0)
            return C(x, x + y);
        return C(x + y, x + y);
    }
    if (isinf(x) || isinf(y)) {
        T re = isinf(x) ? (isinf(y) ? K_PIO4 : K_PIO2) : 0.0;
        return C(M(copysign)(re, x), M(copysign)(INF, y));
    }
    if (M(fabs)(x) > K_ASIN_BIG || M(fabs)(y) > K_ASIN_BIG) {
        /* asin z = -i log(iz + sqrt(1 - z^2)) ~ -i log(2iz), on the branch of
           z's quadrant */
        T re = M(atan2)(M(fabs)(x), M(fabs)(y));
        return C(M(copysign)(re, x), M(copysign)(dt_round(log2z(x, y)), y));
    }
    if (y == 0.0 && M(fabs)(x) <= 1.0)
        return C(M(asin)(x), y);
    s12(x, y, &r1, &i1, &r2, &i2);
    a = dt_sub(dt_mul(r1, r2), dt_mul(i1, i2));
    b = dt_sub(dt_mul(r1, i2), dt_mul(i1, r2));
    im = dt_round(xasinh(b.h < 0.0 ? dt_neg(b) : b));
    return C(dt_round(xatan2(dt(x, 0.0), a)), M(copysign)(im, b.h));
}

CT FN(cacos)(CT z)
{
    T x = RE(z), y = IM(z);
    DT r1, i1, r2, i2, b;
    T im;
    if (isnan(x) || isnan(y)) {
        if (isinf(x))
            return C(x + y, INF);
        if (isinf(y))
            return C(x + y, -y);
        if (x == 0.0)
            return C(K_PIO2, x + y);
        return C(x + y, x + y);
    }
    if (isinf(x) || isinf(y)) {
        T re;
        if (isinf(x) && isinf(y))
            re = x > 0.0 ? K_PIO4 : K_3PIO4;
        else if (isinf(x))
            re = x > 0.0 ? 0.0 : K_PI;
        else
            re = K_PIO2;
        return C(re, M(copysign)(INF, -y));
    }
    if (M(fabs)(x) > K_ASIN_BIG || M(fabs)(y) > K_ASIN_BIG) {
        /* pi/2 - asin z: the real part atan2(|y|, x) (pi/2 - atan2(|x|, |y|)
           would cancel) */
        return C(M(atan2)(M(fabs)(y), x), M(copysign)(dt_round(log2z(x, y)), -y));
    }
    if (y == 0.0 && M(fabs)(x) <= 1.0)
        return C(M(acos)(x), -y);
    s12(x, y, &r1, &i1, &r2, &i2);
    b = dt_sub(dt_mul(r2, i1), dt_mul(i2, r1));
    im = dt_round(xasinh(b.h < 0.0 ? dt_neg(b) : b));
    {
        DT a = xatan2(r1, r2);
        return C(dt_round(dt(2.0 * a.h, 2.0 * a.l)), M(copysign)(im, b.h));
    }
}

/* asinh z = -i asin(iz), acosh z = +-i acos z (G.6.2.1-2) */
CT FN(casinh)(CT z)
{
    CT w = FN(casin)(C(-IM(z), RE(z)));
    return C(IM(w), -RE(w));
}

CT FN(cacosh)(CT z)
{
    CT w = FN(cacos)(z);
    /* (the real part of the result is not negative) */
    if (signbit(IM(w)) || (isnan(IM(w)) && !signbit(IM(z))))
        return C(-IM(w), RE(w));
    return C(IM(w), -RE(w));
}

/* atanh z (G.6.2.3): Kahan's form,
   re = log1p(4x / ((1 - x)^2 + y^2)) / 4, im = atan2(2y, (1 - x)(1 + x) - y^2) / 2 */
CT FN(catanh)(CT z)
{
    T x = RE(z), y = IM(z), ax = M(fabs)(x), ay = M(fabs)(y), re, im;
    if (isnan(x) || isnan(y)) {
        if (isinf(x) || isinf(y))
            return C(M(copysign)(0.0, x), M(copysign)(K_PIO2, y));
        if (x == 0.0)
            return C(x, x + y);
        return C(x + y, x + y);
    }
    if (isinf(x) || isinf(y))
        return C(M(copysign)(0.0, x), M(copysign)(K_PIO2, y));
    if (ax == 1.0 && y == 0.0) {
        volatile T zero = 0.0;
        return C(x / zero, y); /* +-inf, "divide-by-zero" */
    }
    if (ax > K_ATANH_BIG || ay > K_ATANH_BIG) {
        /* Re(1/z) = x / |z|^2, scaled; im = +-pi/2 */
        int e = M(ilogb)(ax > ay ? ax : ay);
        T u = M(scalbn)(ax, -e), v = M(scalbn)(ay, -e);
        DT q = dt_div(dt(u, 0.0), dt_add(dt_prod(u, u), dt_prod(v, v)));
        re = M(scalbn)(dt_round(q), -e);
        return C(M(copysign)(re, x), M(copysign)(K_PIO2, y));
    }
    if (y == 0.0 && ax < 1.0)
        return C(M(atanh)(x), y);
    {
        /* (1 - |x|)^2 + y^2, (1 + |x|)^2 + y^2 and (1 - |x|)(1 + |x|) - y^2
           in pairs (1 -+ |x| exact as pairs) */
        DT m = dt_sum(1.0, -ax), p = dt_sum(1.0, ax), y2 = dt_prod(ay, ay);
        DT den = dt_add(dt_mul(m, m), y2), b = dt_sub(dt_mul(m, p), y2);
        if (ax >= 0.5 && ax <= 2.0 && den.h < 4.0 * ax) {
            /* the quotient beyond 1 (it may overflow near +-1): log((1 +
               |x|)^2 + y^2) - log((1 - |x|)^2 + y^2), 1 - |x| exact */
            DT l1 = xlog(dt_add(dt_mul(p, p), y2));
            DT l2 = xhalflog(M(fabs)(m.h), ay);
            l2 = dt(2.0 * l2.h, 2.0 * l2.l);
            re = dt_round(dt_half(dt_half(dt_sub(l1, l2))));
        } else {
            re = dt_round(dt_half(dt_half(xlog1p(dt_div(dt(4.0 * ax, 0.0), den)))));
        }
        im = dt_round(dt_half(xatan2(dt(2.0 * ay, 0.0), b)));
    }
    return C(M(copysign)(re, x), M(copysign)(im, y));
}

/* atan z = -i atanh(iz) (G.6) */
CT FN(catan)(CT z)
{
    CT w = FN(catanh)(C(-IM(z), RE(z)));
    return C(IM(w), -RE(w));
}

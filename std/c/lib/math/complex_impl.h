/* complex_impl.h - the functions of <complex.h> (C17 7.3.5-7.3.9), written
   once for double (complex.c) and long double (complexl.c), with the
   special values of Annex G (G.6) and the methods of W. Kahan, "Branch cuts
   for complex elementary functions, or much ado about nothing's sign bit"
   (1987): csqrt without overflow, casin and cacos from the square roots of
   1 - z and 1 + z, catanh with log1p, ctanh in the form that keeps its
   accuracy for large real parts. Each part is computed from the real
   functions of the type; the results are accurate to a few ulps (docs/c.md
   gives the measured errors), not correctly rounded.

   The including unit defines T (the real type), CT (the complex type), C,
   RE, IM, INF, FN(n) (this file's functions' names: cexp, or cexpl), M(n)
   (the real functions': exp, or expl), clog_re(ax, ay) (log|z| for ax >= ay
   >= 0, ax > 0 finite, without cancellation near |z| = 1), and the
   constants: K_MAX (the largest finite value), K_EXP_BIG (where e^x is
   near overflow), K_TINY, K_UP and K_DOWN_HALF (csqrt's scaling of tiny
   parts: K_UP K_DOWN_HALF^2 = 1), K_TANH_BIG (where tanh x is 1 to the
   type's precision), K_ASIN_BIG (beyond which 1 +- z loses z's low bits),
   K_ATANH_BIG (where 1/z is catanh's), and pi/4, pi/2, 3pi/4, pi and ln 2
   (K_PIO4, K_PIO2, K_3PIO4, K_PI, K_LN2). */

/* Pairs of T (h + l, |l| <= ulp(h)/2: twice T's precision) for the steps
   where the roundings of T would add up (Dekker's and Knuth's error-free
   transformations: T. J. Dekker, "A floating-point technique for extending
   the available precision", Numer. Math. 18, 1971; the products' errors
   from fma). */
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
    T s = a + b, bb = s - a;
    return dt(s, (a - (s - bb)) + (b - bb));
}

static DT dt_prod(T a, T b)
{
    T p = a * b;
    return dt(p, M(fma)(a, b, -p));
}

static DT dt_add(DT a, DT b)
{
    DT s = dt_sum(a.h, b.h);
    T l = s.l + (a.l + b.l);
    return dt_sum(s.h, l);
}

static DT dt_mul(DT a, DT b)
{
    DT p = dt_prod(a.h, b.h);
    T l = p.l + (a.h * b.l + a.l * b.h);
    return dt_sum(p.h, l);
}

static DT dt_div(DT a, DT b)
{
    T q = a.h / b.h;
    DT r = dt_add(a, dt_mul(b, dt(-q, 0.0)));
    return dt_sum(q, (r.h + r.l) / b.h);
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

/* e^z (G.6.3.1) */
CT FN(cexp)(CT z)
{
    T x = RE(z), y = IM(z), e, c, s;
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
    c = M(cos)(y);
    s = M(sin)(y);
    if (x > K_EXP_BIG) {
        /* e^x overflows before its product with cos y, sin y might not */
        e = M(exp)(0.5 * x);
        return C((e * c) * e, (e * s) * e);
    }
    e = M(exp)(x);
    return C(e * c, e * s);
}

/* log z (G.6.3.2): log|z| + i arg z */
CT FN(clog)(CT z)
{
    T x = RE(z), y = IM(z), ax = M(fabs)(x), ay = M(fabs)(y), r;
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
    if (ay > ax) {
        T t = ax;
        ax = ay;
        ay = t;
    }
    r = clog_re(ax, ay); /* log|z|, ax >= ay */
    return C(r, M(atan2)(y, x));
}

/* the principal square root (G.6.4.2): Kahan's, scaled against overflow */
CT FN(csqrt)(CT z)
{
    T x = RE(z), y = IM(z), t, ax, ay, sc = 1.0;
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
    ax = M(fabs)(x);
    ay = M(fabs)(y);
    if (ax > K_MAX / 4 || ay > K_MAX / 4) {
        ax *= 0.25;
        ay *= 0.25;
        sc = 2.0;
    } else if (ax < K_TINY && ay < K_TINY) {
        ax *= K_UP;
        ay *= K_UP;
        sc = K_DOWN_HALF;
    }
    {
        /* t = sqrt((ax + |z|) / 2) with the sum exact and one Newton step
           in pairs of T, then ay / 2t (each part about one rounding from
           hypot's) */
        DT h = dt_sum(ax, M(hypot)(ax, ay)), q;
        T t0;
        h.h *= 0.5;
        h.l *= 0.5;
        t0 = M(sqrt)(h.h);
        q = dt_add(h, dt_prod(-t0, t0));
        {
            DT tt = dt_sum(t0, (q.h + q.l) / (2.0 * t0));
            DT v = dt_div(dt(ay, 0.0), dt(2.0 * tt.h, 2.0 * tt.l));
            T re = tt.h + tt.l, im = v.h + v.l;
            (void)t;
            if (x >= 0.0)
                return C(re * sc, M(copysign)(im * sc, y));
            return C(im * sc, M(copysign)(re * sc, y));
        }
    }
}

/* z^w = e^(w log z) (G.6.4.1 leaves the special cases to cexp and clog) */
CT FN(cpow)(CT z, CT w)
{
    CT l;
    if (RE(w) == 0.0 && IM(w) == 0.0)
        return C(1.0, 0.0);
    if (RE(z) == 0.0 && IM(z) == 0.0 && IM(w) == 0.0 && RE(w) > 0.0)
        return C(0.0, 0.0);
    l = FN(clog)(z);
    return FN(cexp)(C(RE(w) * RE(l) - IM(w) * IM(l), RE(w) * IM(l) + IM(w) * RE(l)));
}

/* sinh z (G.6.2.5) */
CT FN(csinh)(CT z)
{
    T x = RE(z), y = IM(z), e;
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
    if (M(fabs)(x) > K_EXP_BIG) {
        /* e^|x| / 2 overflows before its products might not: e^(|x|/2) twice */
        T sg = x < 0.0 ? -0.5 : 0.5;
        e = M(exp)(0.5 * M(fabs)(x));
        return C(((sg * e) * M(cos)(y)) * e, ((0.5 * e) * M(sin)(y)) * e);
    }
    return C(M(sinh)(x) * M(cos)(y), M(cosh)(x) * M(sin)(y));
}

/* cosh z (G.6.2.4) */
CT FN(ccosh)(CT z)
{
    T x = RE(z), y = IM(z), e;
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
    if (M(fabs)(x) > K_EXP_BIG) {
        T sg = x < 0.0 ? -0.5 : 0.5;
        e = M(exp)(0.5 * M(fabs)(x));
        return C(((0.5 * e) * M(cos)(y)) * e, ((sg * e) * M(sin)(y)) * e);
    }
    return C(M(cosh)(x) * M(cos)(y), M(sinh)(x) * M(sin)(y));
}

/* tanh z (G.6.2.6): Kahan's form, t = tan y, s = sinh x:
   ((1 + t^2) s sqrt(1 + s^2) + i t) / (1 + (1 + t^2) s^2) */
CT FN(ctanh)(CT z)
{
    T x = RE(z), y = IM(z), t, b, s, r, d;
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
    if (M(fabs)(x) > K_TANH_BIG) {
        /* tanh x is 1 to T precision; the imaginary part 4 sin y cos y
           e^(-2|x|), which may underflow */
        /* (2 sin(2y) e^(-2|x|): one exponential, one sine) */
        T sy = M(fabs)(y) < K_MAX / 4 ? 2.0 * M(sin)(2.0 * y) : 4.0 * M(sin)(y) * M(cos)(y);
        return C(M(copysign)(1.0, x), sy * M(exp)(-2.0 * M(fabs)(x)));
    }
    /* Kahan's: t = tan y, b = 1 + t^2, s = sinh x, r = cosh x,
       tanh z = (b r s + i t) / (1 + b s^2), b and 1 + b s^2 in pairs of T
       (each part's error then mostly that of tan, sinh and cosh) */
    t = M(tan)(y);
    s = M(sinh)(x);
    r = M(cosh)(x);
    {
        DT bb = dt_add(dt_prod(t, t), dt(1.0, 0.0));
        DT dd = dt_add(dt_mul(bb, dt_prod(s, s)), dt(1.0, 0.0));
        DT re = dt_div(dt_mul(dt_mul(bb, dt(r, 0.0)), dt(s, 0.0)), dd);
        DT im = dt_div(dt(t, 0.0), dd);
        (void)b;
        (void)d;
        return C(re.h + re.l, im.h + im.l);
    }
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
   asin z = atan(re z / re(s1 s2)) + i asinh(im(conj(s1) s2)). For |z| large
   (where 1 +- z would lose z's low bits, or s1 s2 overflow) they are
   log(2z)-based. */
static CT big_asin(CT z)
{
    /* asin z = -i log(iz + sqrt(1 - z^2)) ~ -i log(2iz) (|z| > K_ASIN_BIG), on the
       branch of z's quadrant */
    T x = RE(z), y = IM(z);
    CT l = FN(clog)(C(M(fabs)(x), M(fabs)(y)));
    T re = M(atan2)(M(fabs)(x), M(fabs)(y)), im = RE(l) + K_LN2;
    return C(M(copysign)(re, x), M(copysign)(im, y));
}

CT FN(casin)(CT z)
{
    T x = RE(z), y = IM(z);
    CT s1, s2;
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
    if (M(fabs)(x) > K_ASIN_BIG || M(fabs)(y) > K_ASIN_BIG)
        return big_asin(z);
    if (y == 0.0 && M(fabs)(x) <= 1.0)
        return C(M(asin)(x), y);
    s1 = FN(csqrt)(C(1.0 - x, -y));
    s2 = FN(csqrt)(C(1.0 + x, y));
    {
        /* (the products' differences, which may cancel, in pairs of T) */
        DT a = dt_add(dt_prod(RE(s1), RE(s2)), dt_prod(-IM(s1), IM(s2)));
        DT b = dt_add(dt_prod(RE(s1), IM(s2)), dt_prod(-IM(s1), RE(s2)));
        return C(M(atan2)(x, a.h + a.l), M(asinh)(b.h + b.l));
    }
}

CT FN(cacos)(CT z)
{
    T x = RE(z), y = IM(z);
    CT s1, s2;
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
        CT a = big_asin(z);
        return C(K_PIO2 - RE(a), -IM(a));
    }
    if (y == 0.0 && M(fabs)(x) <= 1.0)
        return C(M(acos)(x), -y);
    s1 = FN(csqrt)(C(1.0 - x, -y));
    s2 = FN(csqrt)(C(1.0 + x, y));
    {
        DT b = dt_add(dt_prod(RE(s2), IM(s1)), dt_prod(-IM(s2), RE(s1)));
        return C(2.0 * M(atan2)(RE(s1), RE(s2)), M(asinh)(b.h + b.l));
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
        /* 1/z: re = x / |z|^2, im = +-pi/2 */
        T s = ax > ay ? ax : ay;
        T u = x / s, v = y / s;
        re = (u / (u * u + v * v)) / s;
        return C(re, M(copysign)(K_PIO2, y));
    }
    if (y == 0.0 && ax < 1.0)
        return C(M(atanh)(x), y);
    {
        /* (1 - ax)^2 + ay^2 and (1 - ax)(1 + ax) - ay^2 in pairs of T */
        DT m = dt_sum(1.0, -ax), p = dt_sum(1.0, ax), y2 = dt_prod(ay, ay);
        DT den = dt_add(dt_mul(m, m), y2);
        DT q = dt_div(dt(4.0 * ax, 0.0), den);
        DT b = dt_add(dt_mul(m, p), dt(-y2.h, -y2.l));
        re = 0.25 * M(log1p)(q.h + q.l);
        im = 0.5 * M(atan2)(2.0 * ay, b.h + b.l);
    }
    return C(M(copysign)(re, x), M(copysign)(im, y));
}

/* atan z = -i atanh(iz) (G.6) */
CT FN(catan)(CT z)
{
    CT w = FN(catanh)(C(-IM(z), RE(z)));
    return C(IM(w), -RE(w));
}

/* flags: --c-std=c99 */
/* Numeric code without a math library: Gauss-Jordan inversion with
   partial pivoting of a Hilbert-like matrix and a check that A * inv(A) is
   the identity; a fourth-order Runge-Kutta integrator for a harmonic
   oscillator and a predator-prey system; square roots by Newton's method
   on doubles, bit by bit on integers and through the exponent field;
   exp and log by series with argument reduction; fixed-point Q16.16
   arithmetic with a CORDIC sine and cosine; and the conversion of doubles
   to decimal text written from scratch (exact digits by big decimal
   arithmetic on the binary significand, then rounding to N digits). */
#include "host.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <float.h>

/* ---- linear algebra ---- */

#define N 6
typedef double mat[N][N];

static int invert(mat a, mat inv) {
    mat m;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            m[i][j] = a[i][j];
            inv[i][j] = i == j;
        }
    int swaps = 0;
    for (int c = 0; c < N; c++) {
        int p = c;
        double best = m[c][c] < 0 ? -m[c][c] : m[c][c];
        for (int r = c + 1; r < N; r++) {
            double v = m[r][c] < 0 ? -m[r][c] : m[r][c];
            if (v > best) { best = v; p = r; }
        }
        if (best == 0) return -1;
        if (p != c) {
            swaps++;
            for (int j = 0; j < N; j++) {
                double t = m[c][j]; m[c][j] = m[p][j]; m[p][j] = t;
                t = inv[c][j]; inv[c][j] = inv[p][j]; inv[p][j] = t;
            }
        }
        double d = m[c][c];
        for (int j = 0; j < N; j++) { m[c][j] /= d; inv[c][j] /= d; }
        for (int r = 0; r < N; r++) {
            if (r == c) continue;
            double f = m[r][c];
            if (f == 0) continue;
            for (int j = 0; j < N; j++) {
                m[r][j] -= f * m[c][j];
                inv[r][j] -= f * inv[c][j];
            }
        }
    }
    return swaps;
}

static double max_identity_error(mat a, mat b) {
    double e = 0;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            double s = 0;
            for (int k = 0; k < N; k++) s += a[i][k] * b[k][j];
            double d = s - (i == j);
            if (d < 0) d = -d;
            if (d > e) e = d;
        }
    return e;
}

/* ---- ODEs ---- */

typedef struct { double v[2]; } state;
typedef state (*deriv_fn)(double t, state s, const double *params);

static state axpy(state a, double k, state b) {
    state r;
    for (int i = 0; i < 2; i++) r.v[i] = a.v[i] + k * b.v[i];
    return r;
}

static state rk4(deriv_fn f, double t, state s, double h, const double *p) {
    state k1 = f(t, s, p);
    state k2 = f(t + h / 2, axpy(s, h / 2, k1), p);
    state k3 = f(t + h / 2, axpy(s, h / 2, k2), p);
    state k4 = f(t + h, axpy(s, h, k3), p);
    state r;
    for (int i = 0; i < 2; i++) r.v[i] = s.v[i] + h / 6 * (k1.v[i] + 2 * k2.v[i] + 2 * k3.v[i] + k4.v[i]);
    return r;
}

static state oscillator(double t, state s, const double *p) {
    (void)t;
    return (state){ { s.v[1], -p[0] * s.v[0] } };
}

static state lotka(double t, state s, const double *p) {
    (void)t;
    state r = { { p[0] * s.v[0] - p[1] * s.v[0] * s.v[1], p[2] * s.v[0] * s.v[1] - p[3] * s.v[1] } };
    return r;
}

/* ---- roots, exp, log ---- */

static double newton_sqrt(double x, int *iters) {
    if (x <= 0) return 0;
    double g = x > 1 ? x : 1;
    *iters = 0;
    for (;;) {
        double n = (g + x / g) / 2;
        ++*iters;
        if (n >= g) return g;   /* monotone from above: stop when it stops falling */
        g = n;
    }
}

static uint32_t isqrt64(uint64_t x) {
    uint64_t r = 0, bit = (uint64_t)1 << 62;
    while (bit > x) bit >>= 2;
    while (bit) {
        if (x >= r + bit) {
            x -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return (uint32_t)r;
}

union dbits { double d; uint64_t u; };

/* an initial guess from halving the exponent, then three Newton steps */
static double exp_sqrt(double x) {
    union dbits b = { .d = x };
    b.u = (b.u >> 1) + ((uint64_t)0x3ff << 51);
    double g = b.d;
    for (int i = 0; i < 4; i++) g = 0.5 * (g + x / g);
    return g;
}

static const double LN2 = 0.6931471805599453094;

static double my_exp(double x) {
    int k = (int)(x / LN2 + (x < 0 ? -0.5 : 0.5));
    double r = x - k * LN2, term = 1, sum = 1;
    for (int n = 1; n < 30; n++) {
        term *= r / n;
        sum += term;
    }
    union dbits b = { .d = sum };
    b.u += (uint64_t)(int64_t)k << 52;
    return b.d;
}

static double my_log(double x) {
    union dbits b = { .d = x };
    int e = (int)((b.u >> 52) & 0x7ff) - 1023;
    b.u = (b.u & 0x000fffffffffffffull) | 0x3ff0000000000000ull;
    double m = b.d;   /* 1 <= m < 2 */
    if (m > 1.4142135623730951) { m /= 2; e++; }
    double s = (m - 1) / (m + 1), s2 = s * s, term = s, sum = 0;
    for (int n = 1; n < 60; n += 2) {
        sum += term / n;
        term *= s2;
    }
    return 2 * sum + e * LN2;
}

/* ---- fixed point ---- */

typedef int32_t q16;
#define Q(x) ((q16)((x) * 65536.0))

static q16 qmul(q16 a, q16 b) { return (q16)(((int64_t)a * b) >> 16); }
static q16 qdiv(q16 a, q16 b) { return (q16)(((int64_t)a * 65536) / b); }

static const q16 atan_table[16] = {
    51472, 30386, 16055, 8150, 4091, 2047, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2,
};

static void cordic(q16 angle, q16 *s, q16 *c) {
    q16 x = 39797, y = 0, z = angle;   /* 39797 = 0.607253 * 65536 */
    for (int i = 0; i < 16; i++) {
        q16 dx = x >> i, dy = y >> i;
        if (z >= 0) { x -= dy; y += dx; z -= atan_table[i]; }
        else { x += dy; y -= dx; z += atan_table[i]; }
    }
    *s = y;
    *c = x;
}

/* ---- decimal formatting ---- */

/* the exact decimal expansion of a positive finite double: significand
   times 2^e as a big decimal number in base 10^9 limbs */
#define DL 140
typedef struct { uint32_t l[DL]; int n; int point; } decimal;   /* value = digits * 10^-point9 */

static void dec_mul_small(decimal *d, uint32_t m) {
    uint64_t carry = 0;
    for (int i = 0; i < d->n; i++) {
        uint64_t p = (uint64_t)d->l[i] * m + carry;
        d->l[i] = (uint32_t)(p % 1000000000u);
        carry = p / 1000000000u;
    }
    if (carry) d->l[d->n++] = (uint32_t)carry;
}

/* divide by 2: multiply by 5 and shift the point by one decimal digit;
   tracked with a digit count instead */
static int format_exact(double x, char *out) {
    union dbits b = { .d = x };
    int ex = (int)((b.u >> 52) & 0x7ff);
    uint64_t mant = b.u & 0x000fffffffffffffull;
    if (ex) mant |= (uint64_t)1 << 52; else ex = 1;
    ex -= 1075;   /* x = mant * 2^ex */
    decimal d;
    d.n = 0;
    while (mant) { d.l[d.n++] = (uint32_t)(mant % 1000000000u); mant /= 1000000000u; }
    int frac_digits = 0;
    if (ex > 0) {
        for (int i = 0; i < ex; i++) dec_mul_small(&d, 2);
    } else {
        for (int i = 0; i < -ex; i++) dec_mul_small(&d, 5);
        frac_digits = -ex;
    }
    /* write all digits */
    char digits[1200];
    int nd = 0;
    for (int i = d.n - 1; i >= 0; i--) {
        uint32_t v = d.l[i];
        char tmp[9];
        for (int k = 8; k >= 0; k--) { tmp[k] = (char)('0' + v % 10); v /= 10; }
        int k = 0;
        if (i == d.n - 1) while (k < 8 && tmp[k] == '0') k++;
        for (; k < 9; k++) digits[nd++] = tmp[k];
    }
    /* insert the point */
    int n = 0;
    int int_digits = nd - frac_digits;
    if (int_digits <= 0) {
        out[n++] = '0';
        out[n++] = '.';
        for (int i = 0; i < -int_digits; i++) out[n++] = '0';
        for (int i = 0; i < nd; i++) out[n++] = digits[i];
    } else {
        for (int i = 0; i < nd; i++) {
            if (i == int_digits) out[n++] = '.';
            out[n++] = digits[i];
        }
    }
    /* trim trailing zeros of the fraction */
    bool has_point = false;
    for (int i = 0; i < n; i++) has_point |= out[i] == '.';
    if (has_point) {
        while (out[n - 1] == '0') n--;
        if (out[n - 1] == '.') n--;
    }
    out[n] = 0;
    return n;
}

/* rounds an exact decimal string to `sig` significant digits, half to even
   on an exact tie, and writes it in scientific form d.ddde[+-]xx */
static int format_sci(const char *exact, int sig, char *out) {
    char dg[1200];
    int nd = 0, point = -1;
    int first = -1;
    for (int i = 0; exact[i]; i++) {
        if (exact[i] == '.') { point = nd; continue; }
        if (first < 0 && exact[i] != '0') first = nd;
        dg[nd++] = exact[i];
    }
    if (point < 0) point = nd;
    if (first < 0) { out[0] = '0'; out[1] = 0; return 1; }
    int exp10 = point - first - 1;
    char r[40];
    for (int i = 0; i < sig; i++) r[i] = first + i < nd ? dg[first + i] : '0';
    int rest = first + sig;
    bool up = false;
    if (rest < nd) {
        if (dg[rest] > '5') up = true;
        else if (dg[rest] == '5') {
            bool more = false;
            for (int i = rest + 1; i < nd; i++) more |= dg[i] != '0';
            up = more || ((r[sig - 1] - '0') & 1);
        }
    }
    if (up) {
        int i = sig - 1;
        while (i >= 0 && r[i] == '9') r[i--] = '0';
        if (i >= 0) r[i]++;
        else {
            for (int k = sig - 1; k > 0; k--) r[k] = r[k - 1];
            r[0] = '1';
            exp10++;
        }
    }
    int n = 0;
    out[n++] = r[0];
    if (sig > 1) {
        out[n++] = '.';
        for (int i = 1; i < sig; i++) out[n++] = r[i];
    }
    out[n++] = 'e';
    out[n++] = exp10 < 0 ? '-' : '+';
    int a = exp10 < 0 ? -exp10 : exp10;
    if (a >= 100) out[n++] = (char)('0' + a / 100);
    out[n++] = (char)('0' + a / 10 % 10);
    out[n++] = (char)('0' + a % 10);
    out[n] = 0;
    return n;
}

static void show_sci(double x, int sig) {
    char exact[1300], sci[48];
    int neg = x < 0;
    format_exact(neg ? -x : x, exact);
    int n = format_sci(exact, sig, sci + neg);
    if (neg) sci[0] = '-';
    print_str(sci, (size_t)(n + neg));
}

static uint32_t hash_text(uint32_t h, const char *s) {
    while (*s) h = (h ^ (unsigned char)*s++) * 16777619u;
    return h;
}

int main(void) {
    /* inversion */
    mat a, inv;
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) a[i][j] = 1.0 / (i + j + 1) + (i == j ? 0.5 : 0) + (i == 2 && j == 4 ? 3 : 0);
    int swaps = invert(a, inv);
    print_i64(swaps);
    print_bool(max_identity_error(a, inv) < 1e-9);
    show_sci(inv[0][0], 12);
    show_sci(inv[N - 1][1], 12);

    /* oscillator: after one period the state returns (energy conserved) */
    {
        double k[1] = { 4.0 };
        state s = { { 1.0, 0.0 } };
        double t = 0, h = 0.001;
        int steps = 0;
        double pi = 3.14159265358979323846;
        while (t < pi - h / 2) { s = rk4(oscillator, t, s, h, k); t += h; steps++; }
        print_i64(steps);
        show_sci(s.v[0], 9);
        show_sci(s.v[1], 6);
        double energy = s.v[1] * s.v[1] + 4 * s.v[0] * s.v[0];
        print_bool(energy > 3.9999999 && energy < 4.0000001);
    }
    {
        double p[4] = { 1.1, 0.4, 0.1, 0.4 };
        state s = { { 10, 10 } };
        double maxprey = 0;
        for (int i = 0; i < 5000; i++) {
            s = rk4(lotka, i * 0.01, s, 0.01, p);
            if (s.v[0] > maxprey) maxprey = s.v[0];
        }
        show_sci(s.v[0], 10);
        show_sci(s.v[1], 10);
        show_sci(maxprey, 8);
    }

    /* roots */
    {
        int iters = 0, total = 0;
        double err = 0;
        for (int i = 1; i <= 200; i++) {
            double x = i * 1.37;
            double r = newton_sqrt(x, &iters);
            total += iters;
            double e = r * r - x;
            if (e < 0) e = -e;
            if (e / x > err) err = e / x;
            double r2 = exp_sqrt(x);
            if (r2 != r && (r2 - r > 1e-12 * r || r - r2 > 1e-12 * r)) total += 1000;
        }
        print_i64(total);
        print_bool(err < 4e-16);
        uint64_t sq = 0;
        for (uint64_t v = 1; v < 1000000000000ull; v = v * 3 + 7) sq += isqrt64(v);
        print_u64(sq);
        print_i64(isqrt64(0xffffffffffffffffull));
        print_i64(isqrt64(4000000000000000000ull) == 2000000000u);
    }

    /* exp and log */
    {
        uint32_t h = 2166136261u;
        double worst = 0;
        for (int i = -20; i <= 20; i++) {
            double x = i * 0.73;
            double e = my_exp(x), l = my_log(e);
            double d = l - x;
            if (d < 0) d = -d;
            if (d > worst) worst = d;
            char buf[1300];
            format_exact(e, buf);
            char sci[48];
            format_sci(buf, 15, sci);
            h = hash_text(h, sci);
        }
        print_bool(worst < 1e-13);
        print_u64(h);
        show_sci(my_exp(1.0), 16);
        show_sci(my_log(10.0), 16);
    }

    /* fixed point */
    {
        q16 s, c;
        int64_t acc = 0;
        for (int deg = -90; deg <= 90; deg += 15) {
            q16 ang = qdiv(Q(deg), Q(180)) ;
            ang = qmul(ang, Q(3.14159265));
            cordic(ang, &s, &c);
            acc += s * 3 + c;
            if (deg % 45 == 0) print_i64((int64_t)s * 100000 / 65536);
        }
        print_i64(acc);
        print_i64(qmul(Q(1.5), Q(-2.25)));
        print_i64(qdiv(Q(1), Q(3)));
    }

    /* exact decimal text of some doubles */
    {
        static const double xs[] = { 0.1, 1.0 / 3, 2.5, 1e23, 5e-324, DBL_MAX, 123456789.125, 0.3 + 0.6 * 0.5 };
        uint32_t h = 2166136261u;
        for (int i = 0; i < (int)(sizeof xs / sizeof xs[0]); i++) {
            char buf[1300];
            int n = format_exact(xs[i], buf);
            h = hash_text(h, buf);
            if (n <= 60) print_str(buf, (size_t)n);
            else print_i64(n);
            show_sci(xs[i], 17);
        }
        print_u64(h);
        show_sci(-0.000123456789, 4);
        show_sci(9.9999996, 6);
        show_sci(0.125, 2);
        show_sci(0.375, 2);
        print_f64(1.0 / 7);
        print_f64(2e-7);
        print_f64(-1.5e300);
        float f = 16777217.0f;
        print_f64(f);
        print_f64((float)0.1);
        print_f64(FLT_EPSILON);
    }
    return 0;
}

/* expect: 1 */
/* expect: true */
/* expect: 7.53475423720e-01 */
/* expect: -3.66068144468e-01 */
/* expect: 3142 */
/* expect: 9.99999668e-01 */
/* expect: -1.62939e-03 */
/* expect: true */
/* expect: 7.484073069e-02 */
/* expect: 8.694301634e-01 */
/* expect: 2.9067218e+01 */
/* expect: 1828 */
/* expect: true */
/* expect: 1539980 */
/* expect: 4294967295 */
/* expect: 1 */
/* expect: true */
/* expect: 530147116 */
/* expect: 2.718281828459045e+00 */
/* expect: 2.302585092994045e+00 */
/* expect: -100000 */
/* expect: -70712 */
/* expect: 3 */
/* expect: 70710 */
/* expect: 100000 */
/* expect: 497814 */
/* expect: -221184 */
/* expect: 21845 */
/* expect: 0.1000000000000000055511151231257827021181583404541015625 */
/* expect: 1.0000000000000001e-01 */
/* expect: 0.333333333333333314829616256247390992939472198486328125 */
/* expect: 3.3333333333333331e-01 */
/* expect: 2.5 */
/* expect: 2.5000000000000000e+00 */
/* expect: 99999999999999991611392 */
/* expect: 9.9999999999999992e+22 */
/* expect: 1076 */
/* expect: 4.9406564584124654e-324 */
/* expect: 309 */
/* expect: 1.7976931348623157e+308 */
/* expect: 123456789.125 */
/* expect: 1.2345678912500000e+08 */
/* expect: 0.59999999999999997779553950749686919152736663818359375 */
/* expect: 5.9999999999999998e-01 */
/* expect: 2750214808 */
/* expect: -1.235e-04 */
/* expect: 1.00000e+01 */
/* expect: 1.2e-01 */
/* expect: 3.8e-01 */
/* expect: 0.14285714285714285 */
/* expect: 2e-7 */
/* expect: -1.5e+300 */
/* expect: 16777216 */
/* expect: 0.10000000149011612 */
/* expect: 1.1920928955078125e-7 */

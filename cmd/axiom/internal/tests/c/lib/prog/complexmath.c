/* flags: --c-std=c11 */
/* Numerics: an n-body simulation in double and long double, a small ray
   tracer printing an ASCII image, complex arithmetic (the functions of
   <complex.h>, roots of unity, a Mandelbrot set, an FFT) and <tgmath.h>'s
   choice of function by type, with the results of <math.h>. The output was
   compared with the system's compiler and library at -O0 and -O2; what
   depends on long double's format is not printed. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <complex.h>
#include <tgmath.h>

/* ---- n-body ---- */
typedef struct { double x, y, z, vx, vy, vz, m; } body;
typedef struct { long double x, y, z, vx, vy, vz, m; } lbody;
#define PI 3.141592653589793
#define SOLAR (4 * PI * PI)
#define DPY 365.24

static body bodies[5] = {
    { 0, 0, 0, 0, 0, 0, SOLAR },
    { 4.84143144246472090e+00, -1.16032004402742839e+00, -1.03622044471123109e-01, 1.66007664274403694e-03 * DPY,
      7.69901118419740425e-03 * DPY, -6.90460016972063023e-05 * DPY, 9.54791938424326609e-04 * SOLAR },
    { 8.34336671824457987e+00, 4.12479856412430479e+00, -4.03523417114321381e-01, -2.76742510726862411e-03 * DPY,
      4.99852801234917238e-03 * DPY, 2.30417297573763929e-05 * DPY, 2.85885980666130812e-04 * SOLAR },
    { 1.28943695621391310e+01, -1.51111514016986312e+01, -2.23307578892655734e-01, 2.96460137564761618e-03 * DPY,
      2.37847173959480950e-03 * DPY, -2.96589568540237556e-05 * DPY, 4.36624404335156298e-05 * SOLAR },
    { 1.53796971148509165e+01, -2.59193146099879641e+01, 1.79258772950371181e-01, 2.68067772490389322e-03 * DPY,
      1.62824170038242295e-03 * DPY, -9.51592254519715870e-05 * DPY, 5.15138902046611451e-05 * SOLAR },
};

static void offset(body *b, int n) {
    double px = 0, py = 0, pz = 0;
    for (int i = 0; i < n; i++) { px += b[i].vx * b[i].m; py += b[i].vy * b[i].m; pz += b[i].vz * b[i].m; }
    b[0].vx = -px / SOLAR; b[0].vy = -py / SOLAR; b[0].vz = -pz / SOLAR;
}
static double energy(const body *b, int n) {
    double e = 0;
    for (int i = 0; i < n; i++) {
        e += 0.5 * b[i].m * (b[i].vx * b[i].vx + b[i].vy * b[i].vy + b[i].vz * b[i].vz);
        for (int j = i + 1; j < n; j++) {
            double dx = b[i].x - b[j].x, dy = b[i].y - b[j].y, dz = b[i].z - b[j].z;
            e -= b[i].m * b[j].m / sqrt(dx * dx + dy * dy + dz * dz);
        }
    }
    return e;
}
static void advance(body *b, int n, double dt) {
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            double dx = b[i].x - b[j].x, dy = b[i].y - b[j].y, dz = b[i].z - b[j].z;
            double d2 = dx * dx + dy * dy + dz * dz;
            double mag = dt / (d2 * sqrt(d2));
            b[i].vx -= dx * b[j].m * mag; b[i].vy -= dy * b[j].m * mag; b[i].vz -= dz * b[j].m * mag;
            b[j].vx += dx * b[i].m * mag; b[j].vy += dy * b[i].m * mag; b[j].vz += dz * b[i].m * mag;
        }
    for (int i = 0; i < n; i++) { b[i].x += dt * b[i].vx; b[i].y += dt * b[i].vy; b[i].z += dt * b[i].vz; }
}
static void ladvance(lbody *b, int n, long double dt) {
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            long double dx = b[i].x - b[j].x, dy = b[i].y - b[j].y, dz = b[i].z - b[j].z;
            long double d2 = dx * dx + dy * dy + dz * dz;
            long double mag = dt / (d2 * sqrtl(d2));
            b[i].vx -= dx * b[j].m * mag; b[i].vy -= dy * b[j].m * mag; b[i].vz -= dz * b[j].m * mag;
            b[j].vx += dx * b[i].m * mag; b[j].vy += dy * b[i].m * mag; b[j].vz += dz * b[i].m * mag;
        }
    for (int i = 0; i < n; i++) { b[i].x += dt * b[i].vx; b[i].y += dt * b[i].vy; b[i].z += dt * b[i].vz; }
}

static void nbody(void) {
    body b[5];
    memcpy(b, bodies, sizeof b);
    offset(b, 5);
    printf("energy %.9f\n", energy(b, 5));
    lbody lb[5];
    for (int i = 0; i < 5; i++)
        lb[i] = (lbody){ b[i].x, b[i].y, b[i].z, b[i].vx, b[i].vy, b[i].vz, b[i].m };
    for (int s = 0; s < 2000; s++) { advance(b, 5, 0.01); ladvance(lb, 5, 0.01L); }
    printf("energy %.9f\n", energy(b, 5));
    long double dx = lb[1].x - (long double)b[1].x;
    printf("long double and double agree to 1e-6: %d\n", fabsl(dx) < 1e-6L);
}

/* ---- ray tracer ---- */
typedef struct { double x, y, z; } vec;
static vec v(double x, double y, double z) { return (vec){ x, y, z }; }
static vec add(vec a, vec b) { return v(a.x + b.x, a.y + b.y, a.z + b.z); }
static vec sub(vec a, vec b) { return v(a.x - b.x, a.y - b.y, a.z - b.z); }
static vec scale(vec a, double s) { return v(a.x * s, a.y * s, a.z * s); }
static double dot(vec a, vec b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static vec norm(vec a) { return scale(a, 1 / sqrt(dot(a, a))); }

struct sphere { vec c; double r; double refl; };
static const struct sphere scene[] = {
    { { 0, -1000, 0 }, 999, 0.2 }, { { 0, 0, 5 }, 1, 0.5 }, { { -2, 0.5, 6 }, 1.2, 0.1 }, { { 2, -0.3, 4 }, 0.7, 0.8 },
};
static int hit(vec o, vec d, double *t) {
    int best = -1;
    *t = INFINITY;
    for (int i = 0; i < (int)(sizeof scene / sizeof scene[0]); i++) {
        vec oc = sub(o, scene[i].c);
        double b = dot(oc, d), c = dot(oc, oc) - scene[i].r * scene[i].r;
        double disc = b * b - c;
        if (disc < 0) continue;
        double s = sqrt(disc), t0 = -b - s;
        if (t0 < 1e-6) t0 = -b + s;
        if (t0 > 1e-6 && t0 < *t) { *t = t0; best = i; }
    }
    return best;
}
static double trace(vec o, vec d, int depth) {
    double t;
    int i = hit(o, d, &t);
    if (i < 0) return 0.1 + 0.2 * fmax(d.y, 0.0);
    vec p = add(o, scale(d, t)), n = norm(sub(p, scene[i].c));
    vec light = norm(v(-1, 2, -1));
    double diff = fmax(dot(n, light), 0.0), ts;
    if (hit(add(p, scale(n, 1e-4)), light, &ts) >= 0) diff *= 0.3;
    double c = 0.1 + 0.9 * diff;
    if (depth < 3 && scene[i].refl > 0) {
        vec r = sub(d, scale(n, 2 * dot(d, n)));
        c = c * (1 - scene[i].refl) + scene[i].refl * trace(add(p, scale(n, 1e-4)), r, depth + 1);
    }
    return c;
}
static void raytrace(void) {
    static const char shades[] = " .:-=+*#%@";
    double total = 0;
    for (int y = 0; y < 18; y++) {
        char row[61];
        for (int x = 0; x < 60; x++) {
            vec d = norm(v((x - 30) / 30.0, (9 - y) / 18.0, 1));
            double c = trace(v(0, 0, 0), d, 0);
            total += c;
            int k = (int)(c * 9.999);
            row[x] = shades[k < 0 ? 0 : k > 9 ? 9 : k];
        }
        row[60] = 0;
        printf("|%s|\n", row);
    }
    printf("total %.6f\n", total);
}

/* ---- complex ---- */
static void fft(double complex *a, int n, int inv) {
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { double complex t = a[i]; a[i] = a[j]; a[j] = t; }
    }
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2 * PI / len * (inv ? -1 : 1);
        double complex wl = cexp(I * ang);
        for (int i = 0; i < n; i += len) {
            double complex w = 1;
            for (int j = 0; j < len / 2; j++) {
                double complex u = a[i + j], t = a[i + j + len / 2] * w;
                a[i + j] = u + t; a[i + j + len / 2] = u - t;
                w *= wl;
            }
        }
    }
    if (inv) for (int i = 0; i < n; i++) a[i] /= n;
}
static void complexes(void) {
    double complex z = 3 + 4 * I;
    printf("|z| %g arg %.6f conj %g%+gi proj %g\n", cabs(z), carg(z), creal(conj(z)), cimag(conj(z)), creal(cproj(z)));
    double complex s = csqrt(-4.0 + 0 * I), l = clog(z), e = cexp(l), p = cpow(z, 2);
    printf("csqrt(-4) %g%+gi clog %.6f%+.6fi exp(log) %.6f%+.6fi pow %.4f%+.4fi\n", creal(s), cimag(s), creal(l), cimag(l), creal(e), cimag(e), creal(p), cimag(p));
    double complex t[] = { csin(z), ccos(z), ctan(z), casin(z), cacos(z), catan(z), csinh(z), ccosh(z), ctanh(z), casinh(z), cacosh(z), catanh(z) };
    for (size_t i = 0; i < sizeof t / sizeof t[0]; i++) printf("%.6g%+.6gi%c", creal(t[i]), cimag(t[i]), i % 4 == 3 ? '\n' : ' ');
    float complex fz = 1.5f - 2.0f * I;
    long double complex lz = 1.0L + 1.0L * I;
    printf("float %g%+gi ld %Lg%+Lgi |lz| %.15Lf\n", crealf(fz * fz), cimagf(fz * fz), creall(lz * lz), cimagl(lz * lz), cabsl(lz));
    double complex q = (1e300 + 1e300 * I) / (1e300 + 1e-300 * I);
    printf("big division %.6g%+.6gi\n", creal(q), cimag(q));
    /* roots of unity sum to zero */
    double complex sum = 0;
    for (int k = 0; k < 12; k++) sum += cexp(2 * PI * I * k / 12);
    printf("roots sum small %d\n", cabs(sum) < 1e-12);
    /* FFT of a known signal */
    double complex a[64];
    for (int i = 0; i < 64; i++) a[i] = sin(2 * PI * 5 * i / 64) + 0.5 * cos(2 * PI * 12 * i / 64);
    fft(a, 64, 0);
    for (int i = 0; i < 64; i++) if (cabs(a[i]) > 1e-9) printf("bin %d %.6f\n", i, cabs(a[i]));
    fft(a, 64, 1);
    printf("inverse %.9f %.9f\n", creal(a[1]), creal(a[2]));
    /* Mandelbrot */
    for (int y = -6; y <= 6; y++) {
        char row[50];
        for (int x = 0; x < 48; x++) {
            double complex c = (-2.2 + x * 0.06) + y * 0.12 * I, w = 0;
            int it = 0;
            while (it < 60 && cabs(w) <= 2) { w = w * w + c; it++; }
            row[x] = it == 60 ? '#' : " .:+"[it % 4];
        }
        row[48] = 0;
        printf("|%s|\n", row);
    }
}

/* ---- tgmath ---- */
#define TYPE(x) _Generic((x), float: "float", double: "double", long double: "long double", \
    float complex: "float complex", double complex: "double complex", long double complex: "long double complex", default: "other")
static void tg(void) {
    float f = 2.0f; double d = 2.0; long double ld = 2.0L; int i = 2;
    double complex dc = 2.0 + 0 * I; float complex fc = 2.0f;
    printf("sqrt: %s %s %s %s %s %s\n", TYPE(sqrt(f)), TYPE(sqrt(d)), TYPE(sqrt(ld)), TYPE(sqrt(i)), TYPE(sqrt(dc)), TYPE(sqrt(fc)));
    printf("pow: %s %s %s %s\n", TYPE(pow(f, f)), TYPE(pow(f, d)), TYPE(pow(f, ld)), TYPE(pow(fc, d)));
    printf("fabs(complex) %s, carg %s, creal %s, atan2 %s, fma %s\n", TYPE(fabs(dc)), TYPE(carg(fc)), TYPE(creal(ld)), TYPE(atan2(f, i)), TYPE(fma(f, f, f)));
    printf("values %.7f %.15f %.15Lf %g\n", (double)sqrt(f), sqrt(d), sqrtl(ld), cbrt(27));
    printf("exp %.6f log %.6f log10 %.6f log2 %.6f expm1 %.6g log1p %.6g\n", exp(1.0), log(10.0), log10(1000.0), log2(1024.0), expm1(1e-10), log1p(1e-10));
    printf("sin %.6f cos %.6f tan %.6f asin %.6f acos %.6f atan %.6f\n", sin(1.0), cos(1.0), tan(1.0), asin(0.5), acos(0.5), atan(1.0));
    printf("sinh %.6f cosh %.6f tanh %.6f asinh %.6f acosh %.6f atanh %.6f\n", sinh(1.0), cosh(1.0), tanh(1.0), asinh(1.0), acosh(2.0), atanh(0.5));
    printf("hypot %g erf %.6f erfc %.6f tgamma %g lgamma %.6f\n", hypot(3.0, 4.0), erf(0.5), erfc(0.5), tgamma(5.0), lgamma(10.0));
    printf("floor %g ceil %g trunc %g round %g nearbyint %g rint %g lround %ld llrint %lld\n", floor(-2.5), ceil(-2.5), trunc(-2.5), round(-2.5), nearbyint(-2.5), rint(-3.5), lround(2.5), llrint(2.5));
    int ex; double fr = frexp(48.0, &ex);
    double ip; double fp = modf(-3.75, &ip);
    printf("frexp %g %d ldexp %g modf %g %g scalbn %g ilogb %d logb %g\n", fr, ex, ldexp(0.75, 6), fp, ip, scalbn(1.0, -3), ilogb(1000.0), logb(1000.0));
    printf("fmod %g remainder %g", fmod(7.5, 2.0), remainder(7.5, 2.0));
    int quo; double rq = remquo(7.5, 2.0, &quo);
    printf(" remquo %g %d copysign %g nextafter %a nexttoward %a fdim %g fmax %g fmin %g\n", rq, quo & 7, copysign(3.0, -0.0), nextafter(1.0, 2.0), nexttoward(1.0f, 0.0L), fdim(5.0, 7.0), fmax(NAN, 1.0), fmin(-0.0, 0.0));
    printf("classify: %d %d %d %d %d\n", fpclassify(0.0) == FP_ZERO, fpclassify(1e-310) == FP_SUBNORMAL, fpclassify(1.0) == FP_NORMAL, fpclassify(INFINITY) == FP_INFINITE, fpclassify(NAN) == FP_NAN);
    printf("compare: %d %d %d %d %d %d\n", isgreater(1.0, NAN), isless(1.0, 2.0), islessgreater(1.0, 1.0), isunordered(NAN, 0.0), isgreaterequal(2.0, 2.0), islessequal(3.0, 2.0));
    printf("signbit %d %d isfinite %d isnormal %d\n", signbit(-0.0) != 0, signbit(0.0) != 0, isfinite(DBL_MAX), isnormal(DBL_MIN / 2));
    printf("nan payload is nan %d, fma exact %a\n", isnan(nan("1")), fma(1.0 + 0x1p-52, 1.0 - 0x1p-52, -1.0));
    printf("float limits %a %a %a eps %a\n", FLT_MAX, FLT_MIN, FLT_TRUE_MIN, FLT_EPSILON);
    printf("double limits %a %a eps %a dig %d mant %d\n", DBL_MAX, DBL_MIN, DBL_EPSILON, DBL_DIG, DBL_MANT_DIG);
}

int main(void) {
    nbody();
    raytrace();
    complexes();
    tg();
    return 0;
}
// expect: energy -0.169075164
// expect: energy -0.169071607
// expect: long double and double agree to 1e-6: 1
// expect: |............................................................|
// expect: |............................................................|
// expect: |............................................................|
// expect: |............................................................|
// expect: |..................%%##......................................|
// expect: |...............%%##**++=-...................................|
// expect: |..............##**++==--:..***+++=..........................|
// expect: |..............+++==--:::.++++++===-:........................|
// expect: |..............===--::....*=====---::......-====--:..........|
// expect: |              -:::......------:::.:.    :=-------::         |
// expect: |        ********.........++++++===--****=:::::::::..*       |
// expect: |*******************...***:=======-=.****:**********+********|
// expect: |******************:::::::::-===:.:::::***********++*********|
// expect: |****************************#:::::::*****::****+:::*********|
// expect: |**************************#########*******####--:::*********|
// expect: |*************************###########*****#########**********|
// expect: |*************####*******###########**##############*********|
// expect: |***##################***########**#**#############**********|
// expect: total 414.412745
// expect: |z| 5 arg 0.927295 conj 3-4i proj 3
// expect: csqrt(-4) 0+2i clog 1.609438+0.927295i exp(log) 3.000000+4.000000i pow -7.0000+24.0000i
// expect: 3.85374-27.0168i -27.0349-3.85115i -0.000187346+0.999356i 0.633984+2.30551i
// expect: 0.936812-2.30551i 1.44831+0.158997i -6.54812-7.61923i -6.58066-7.58155i
// expect: 1.00071+0.00490826i 2.29991+0.917617i 2.30551+0.936812i 0.117501+1.40992i
// expect: float -1.75-6i ld 0+2i |lz| 1.414213562373095
// expect: big division 1+1i
// expect: roots sum small 1
// expect: bin 5 32.000000
// expect: bin 12 16.000000
// expect: bin 52 16.000000
// expect: bin 59 32.000000
// expect: inverse 0.662738453 0.477916222
// expect: |......::++++++++++++     ..++++.+####: +:::. +++|
// expect: |.....:++++++++++++   ...:: :# :########   ++: ++|
// expect: |.....+++++++++  ......:::+:###############+ :. +|
// expect: |....:++++    .:++  .: + .###################:.  |
// expect: |....++     ...++. ####. .###################..  |
// expect: |....     .::+++ ########.##################.:.  |
// expect: |....#####################################::+:.  |
// expect: |....     .::+++ ########.##################.:.  |
// expect: |....++     ...++. ####. .###################..  |
// expect: |....:++++    .:++  .: + .###################:.  |
// expect: |.....+++++++++  ......:::+:###############+ :. +|
// expect: |.....:++++++++++++   ...:: :# :########   ++: ++|
// expect: |......::++++++++++++     ..++++.+####: +:::. +++|
// expect: sqrt: float double long double double double complex float complex
// expect: pow: float double long double double complex
// expect: fabs(complex) double, carg float, creal long double, atan2 double, fma float
// expect: values 1.4142135 1.414213562373095 1.414213562373095 3
// expect: exp 2.718282 log 2.302585 log10 3.000000 log2 10.000000 expm1 1e-10 log1p 1e-10
// expect: sin 0.841471 cos 0.540302 tan 1.557408 asin 0.523599 acos 1.047198 atan 0.785398
// expect: sinh 1.175201 cosh 1.543081 tanh 0.761594 asinh 0.881374 acosh 1.316958 atanh 0.549306
// expect: hypot 5 erf 0.520500 erfc 0.479500 tgamma 24 lgamma 12.801827
// expect: floor -3 ceil -2 trunc -2 round -3 nearbyint -2 rint -4 lround 3 llrint 2
// expect: frexp 0.75 6 ldexp 48 modf -0.75 -3 scalbn 0.125 ilogb 9 logb 9
// expect: fmod 1.5 remainder -0.5 remquo -0.5 4 copysign -3 nextafter 0x1.0000000000001p+0 nexttoward 0x1.fffffep-1 fdim 0 fmax 1 fmin -0
// expect: classify: 1 1 1 1 1
// expect: compare: 0 1 0 1 1 0
// expect: signbit 1 0 isfinite 1 isnormal 0
// expect: nan payload is nan 1, fma exact -0x1p-104
// expect: float limits 0x1.fffffep+127 0x1p-126 0x1p-149 eps 0x1p-23
// expect: double limits 0x1.fffffffffffffp+1023 0x1p-1022 eps 0x1p-52 dig 15 mant 53

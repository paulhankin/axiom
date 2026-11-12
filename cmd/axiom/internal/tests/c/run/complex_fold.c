/* Constant folding of complex arithmetic equals its evaluation at run time
   (6.6, Annex G.5): + - * / of every pair of a set of complex values with
   special parts (signed zeros, infinities, NaNs, huge and tiny values),
   and with real operands on either side, in double and float, folded in a
   static initializer and computed from volatile operands. The parts must
   agree bit for bit (any NaN with any NaN). */
#include <complex.h>
#include "host.h"

#define INF (__builtin_inff())
#define NAN (__builtin_nanf(""))
#define CS(X, Y, T, M) \
    X(T, M, Y, 1.0, 2.0) X(T, M, Y, -0.0, 0.0) X(T, M, Y, 0.0, -0.0) X(T, M, Y, 3.0, -4.0) \
    X(T, M, Y, 1e30, 1e30) X(T, M, Y, -1e-30, 2e-30) X(T, M, Y, INF, 0.0) X(T, M, Y, 0.0, -INF) \
    X(T, M, Y, INF, NAN) X(T, M, Y, NAN, INF) X(T, M, Y, NAN, NAN) X(T, M, Y, NAN, 1.0) \
    X(T, M, Y, -INF, INF) X(T, M, Y, 1e-40, -3.0) X(T, M, Y, 2.0, 0.0) X(T, M, Y, 0.0, 0.0)
#define CS2(X, T, M, a, b) \
    X(T, M, a, b, 1.0, 2.0) X(T, M, a, b, -0.0, 0.0) X(T, M, a, b, 0.0, -0.0) X(T, M, a, b, 3.0, -4.0) \
    X(T, M, a, b, 1e30, 1e30) X(T, M, a, b, -1e-30, 2e-30) X(T, M, a, b, INF, 0.0) X(T, M, a, b, 0.0, -INF) \
    X(T, M, a, b, INF, NAN) X(T, M, a, b, NAN, INF) X(T, M, a, b, NAN, NAN) X(T, M, a, b, NAN, 1.0) \
    X(T, M, a, b, -INF, INF) X(T, M, a, b, 1e-40, -3.0) X(T, M, a, b, 2.0, 0.0) X(T, M, a, b, 0.0, 0.0)
#define RS(X, T, M, a, b) X(T, M, a, b, 0.0) X(T, M, a, b, -0.0) X(T, M, a, b, 2.0) X(T, M, a, b, INF) \
    X(T, M, a, b, NAN) X(T, M, a, b, -1e30)
#define NC 16
#define NR 6

/* the complex-complex results of a and b, then the real ones */
#define PAIR(T, M, a, b, c, d) M((T)(a), (T)(b)) + M((T)(c), (T)(d)), M((T)(a), (T)(b)) - M((T)(c), (T)(d)), \
    M((T)(a), (T)(b)) * M((T)(c), (T)(d)), M((T)(a), (T)(b)) / M((T)(c), (T)(d)),
#define MIX(T, M, a, b, r) M((T)(a), (T)(b)) + (T)(r), (T)(r) + M((T)(a), (T)(b)), \
    M((T)(a), (T)(b)) - (T)(r), (T)(r) - M((T)(a), (T)(b)), M((T)(a), (T)(b)) * (T)(r), (T)(r) * M((T)(a), (T)(b)), \
    M((T)(a), (T)(b)) / (T)(r), (T)(r) / M((T)(a), (T)(b)),
#define ROW(T, M, Y, a, b) Y(PAIR, T, M, a, b)
#define MROW(T, M, Y, a, b) RS(MIX, T, M, a, b)
#define PART(T, M, Y, a, b) { (T)(a), (T)(b) },

static const double complex KD[] = { CS(ROW, CS2, double, CMPLX) CS(MROW, 0, double, CMPLX) };
static const float complex KF[] = { CS(ROW, CS2, float, CMPLXF) CS(MROW, 0, float, CMPLXF) };
static volatile double VD[][2] = { CS(PART, 0, double, CMPLX) };
static volatile float VF[][2] = { CS(PART, 0, float, CMPLXF) };
#define RV(T, M, a, b, r) (T)(r),
static volatile double RD[] = { RS(RV, double, 0, 0, 0) };
static volatile float RF[] = { RS(RV, float, 0, 0, 0) };

typedef union { double d; unsigned long long u; } BD;
typedef union { float f; unsigned u; } BF;

static int same_d(double x, double y) {
    BD a, b;
    a.d = x;
    b.d = y;
    return (x != x && y != y) || a.u == b.u;
}

static int same_f(float x, float y) {
    BF a, b;
    a.f = x;
    b.f = y;
    return (x != x && y != y) || a.u == b.u;
}

static unsigned long long hash_d(unsigned long long h, double x) {
    BD a;
    a.d = x;
    if (x != x) return h * 31 + 7;
    return h * 1000003 + a.u;
}

int main(void) {
    int i, j, k = 0, bad = 0;
    unsigned long long h = 0;
    for (i = 0; i < NC; i++) {
        double complex a = CMPLX(VD[i][0], VD[i][1]);
        for (j = 0; j < NC; j++) {
            double complex b = CMPLX(VD[j][0], VD[j][1]), r[4];
            r[0] = a + b;
            r[1] = a - b;
            r[2] = a * b;
            r[3] = a / b;
            for (int o = 0; o < 4; o++, k++) {
                if (!same_d(creal(r[o]), creal(KD[k])) || !same_d(cimag(r[o]), cimag(KD[k]))) bad++;
                h = hash_d(hash_d(h, creal(r[o])), cimag(r[o]));
            }
        }
    }
    for (i = 0; i < NC; i++) {
        double complex a = CMPLX(VD[i][0], VD[i][1]);
        for (j = 0; j < NR; j++) {
            double x = RD[j];
            double complex r[8];
            r[0] = a + x;
            r[1] = x + a;
            r[2] = a - x;
            r[3] = x - a;
            r[4] = a * x;
            r[5] = x * a;
            r[6] = a / x;
            r[7] = x / a;
            for (int o = 0; o < 8; o++, k++) {
                if (!same_d(creal(r[o]), creal(KD[k])) || !same_d(cimag(r[o]), cimag(KD[k]))) bad++;
                h = hash_d(hash_d(h, creal(r[o])), cimag(r[o]));
            }
        }
    }
    print_i64(k == (int)(sizeof KD / sizeof KD[0]));
    print_i64(bad);
    print_u64(h);
    k = 0;
    bad = 0;
    h = 0;
    for (i = 0; i < NC; i++) {
        float complex a = CMPLXF(VF[i][0], VF[i][1]);
        for (j = 0; j < NC; j++) {
            float complex b = CMPLXF(VF[j][0], VF[j][1]), r[4];
            r[0] = a + b;
            r[1] = a - b;
            r[2] = a * b;
            r[3] = a / b;
            for (int o = 0; o < 4; o++, k++) {
                if (!same_f(crealf(r[o]), crealf(KF[k])) || !same_f(cimagf(r[o]), cimagf(KF[k]))) bad++;
                h = hash_d(hash_d(h, crealf(r[o])), cimagf(r[o]));
            }
        }
    }
    for (i = 0; i < NC; i++) {
        float complex a = CMPLXF(VF[i][0], VF[i][1]);
        for (j = 0; j < NR; j++) {
            float x = RF[j];
            float complex r[8];
            r[0] = a + x;
            r[1] = x + a;
            r[2] = a - x;
            r[3] = x - a;
            r[4] = a * x;
            r[5] = x * a;
            r[6] = a / x;
            r[7] = x / a;
            for (int o = 0; o < 8; o++, k++) {
                if (!same_f(crealf(r[o]), crealf(KF[k])) || !same_f(cimagf(r[o]), cimagf(KF[k]))) bad++;
                h = hash_d(hash_d(h, crealf(r[o])), cimagf(r[o]));
            }
        }
    }
    print_i64(k == (int)(sizeof KF / sizeof KF[0]));
    print_i64(bad);
    print_u64(h);
    return 0;
}

/* expect: 1 */
/* expect: 0 */
/* expect: 2134481200256986412 */
/* expect: 1 */
/* expect: 0 */
/* expect: 15013598767432812172 */

/* flags: --c-std=c11 */
/* Counted loops made vector loops (vectorize.ax) at -O2 and -O3, with the
   scalar loop for the rest: element-wise float and integer arithmetic,
   integer reductions (+, -, *, &, |, ^, min and max, signed and unsigned)
   and the cases the vector loop must leave to the scalar one: a store into
   the next elements of what is loaded (a dependence the vector loop would
   break: the destination one, two and three elements after the source),
   counts below a vector, wrapping sums, running sums stored or used again
   in the loop (which stay scalar). Floating-point sums keep their
   order (they are not made vector reductions). Every count from 0 to 70. */
#include "host.h"
static float fa[80], fb[80];
static double da[80];
static int ia[80], ib[80];
static unsigned char ua[96], ub[96];
static short sa[80];
static long long la[80];

static void saxpy(float *y, const float *x, float a, int n) {
    for (int i = 0; i < n; i++)
        y[i] = a * x[i] + y[i];
}
static void dscale(double *y, const double *x, int n) {
    for (int i = 0; i < n; i++)
        y[i] = x[i] / 3.0 - 1.0;
}
static int isum(const int *x, int n) {
    int s = 7;
    for (int i = 0; i < n; i++)
        s += x[i];
    return s;
}
static unsigned usub(const unsigned *x, int n) {
    unsigned s = 0;
    for (int i = 0; i < n; i++)
        s -= x[i];
    return s;
}
static unsigned short smul(const unsigned short *x, int n) {
    unsigned short s = 1;
    for (int i = 0; i < n; i++)
        s *= x[i];
    return s;
}
static int imax(const int *x, int n) {
    int m = -2147483647 - 1;
    for (int i = 0; i < n; i++)
        if (x[i] > m)
            m = x[i];
    return m;
}
static unsigned char umin(const unsigned char *x, int n) {
    unsigned char m = 200;
    for (int i = 0; i < n; i++)
        if (x[i] < m)
            m = x[i];
    return m;
}
static short smin(const short *x, int n) {
    short m = 1000;
    for (int i = 0; i < n; i++)
        m = x[i] <= m ? x[i] : m;
    return m;
}
static long long lxor(const long long *x, int n) {
    long long s = 0, t = -1, u = 0;
    for (int i = 0; i < n; i++)
        s ^= x[i];
    for (int i = 0; i < n; i++)
        t &= x[i];
    for (int i = 0; i < n; i++)
        u |= x[i];
    return s + t * 3 + u * 5;
}
static void addb(unsigned char *d, const unsigned char *a, const unsigned char *b, unsigned long n) {
    for (unsigned long i = 0; i < n; i++)
        d[i] = a[i] + b[i];
}
static void shifts(int *d, const int *a, int n) {
    for (int i = 0; i < n; i++)
        d[i] = (a[i] << 3) ^ (a[i] >> 2);
}
static void prefix(int *d, const int *x, int n) {
    int s = 0;  /* (the running sum is stored: not a vector reduction) */
    for (int i = 0; i < n; i++) {
        s += x[i];
        d[i] = s;
    }
}
static int twice(const int *x, int n) {
    int s = 0, t = 0;  /* (the sum used again in the loop) */
    for (int i = 0; i < n; i++) {
        s += x[i];
        t ^= s;
    }
    return s + t;
}
static float fsum(const float *x, int n) {
    float s = 0.0f;
    for (int i = 0; i < n; i++)
        s += x[i];
    return s;
}
static void reset(void) {
    for (int i = 0; i < 80; i++) {
        fa[i] = (float)(i * 0.37 - 5.0);
        fb[i] = (float)(i * i * 0.01 + 0.5);
        da[i] = i * 1.25 - 30;
        ia[i] = i * 2654435761u;
        ib[i] = (i * 7919) % 1000 - 500;
        sa[i] = (short)(i * 1031 - 20000);
        la[i] = (long long)i * 0x123456789abcdefLL;
    }
    for (int i = 0; i < 96; i++) {
        ua[i] = (unsigned char)(i * 37 + 11);
        ub[i] = (unsigned char)(i * 101 + 3);
    }
}
static unsigned long long h = 1469598103934665603ULL;
static void ck(unsigned long long v) {
    h = (h ^ v) * 1099511628211ULL;
    h ^= h >> 31;
}
static void ckf(double d) {
    union { double d; unsigned long long u; } x;
    x.d = d;
    ck(x.u);
}
int main(void) {
    for (int n = 0; n <= 70; n++) {
        reset();
        saxpy(fb, fa, 1.5f, n);
        for (int i = 0; i < 72; i++)
            ckf(fb[i]);
        reset();
        saxpy(fa + 1, fa, 0.75f, n);  /* the store one element after the load */
        for (int i = 0; i < 72; i++)
            ckf(fa[i]);
        reset();
        saxpy(fa, fa + 2, 2.0f, n);   /* (the load ahead: safe either way) */
        for (int i = 0; i < 72; i++)
            ckf(fa[i]);
        reset();
        dscale(da + 3, da, n);
        for (int i = 0; i < 74; i++)
            ckf(da[i]);
        reset();
        ck(isum(ia, n));
        ck(usub((const unsigned *)ib, n));
        ck(smul((const unsigned short *)sa, n));
        ck(imax(ib, n));
        ck(umin(ua, n));
        ck(smin(sa, n));
        ck(lxor(la, n));
        ckf(fsum(fa, n));
        ck(twice(ib, n));
        prefix(ia + 1, ib, n);
        for (int i = 0; i < 80; i++)
            ck(ia[i]);
        reset();
        addb(ua + 1, ua, ub, n + 9);  /* one element after */
        for (int i = 0; i < 96; i++)
            ck(ua[i]);
        reset();
        addb(ua, ub, ub + 5, n + 9);
        for (int i = 0; i < 96; i++)
            ck(ua[i]);
        reset();
        shifts(ia + 2, ia, n);
        for (int i = 0; i < 80; i++)
            ck(ia[i]);
    }
    print_u64(h);
    return 0;
}
/* expect: 8577096978035050115 */

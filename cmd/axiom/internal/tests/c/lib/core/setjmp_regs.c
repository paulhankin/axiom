/* After longjmp, the objects of the function that called setjmp that were
   not modified since have their values (C17 7.13.2.1p3), however many
   there are (more than registers: integers, doubles and floats, the
   registers a call keeps among them), at every optimization level; and so
   do the caller's. */
/* wasm-flags: --features=+exception-handling */
#include <setjmp.h>
#include "print.h"

static jmp_buf jb;
static volatile int sink;

/* clobbers every register a call may use, then jumps */
static void clobber(int k) {
    volatile double d[16];
    volatile long long x[16];
    int i;
    for (i = 0; i < 16; i++) {
        d[i] = i * 1.5 + k;
        x[i] = (long long)i * 977 + k;
    }
    sink = (int)(d[3] + x[5]);
    longjmp(jb, k + 1);
}

static double work(int n, double scale) {
    long long a = n * 3LL, b = n * 5LL, c = n * 7LL, d = n * 11LL, e = n * 13LL, f = n * 17LL, g = n * 19LL;
    long long h = n * 23LL, i = n * 29LL, j = n * 31LL, k = n * 37LL, l = n * 41LL;
    double x = scale * 1.25, y = scale * 2.5, z = scale * 3.75, w = scale * 5.0, v = scale * 6.25;
    double u = scale * 7.5, t = scale * 8.75, s = scale * 10.0, q = scale * 11.25;
    float p = (float)scale * 0.5f, o = (float)scale * 1.5f;
    int r;
    sink = n;
    r = setjmp(jb);
    if (r == 0) {
        clobber(n);
        return -1.0;
    }
    if (r != n + 1)
        return -2.0;
    return (double)(a + b + c + d + e + f + g + h + i + j + k + l) + x + y + z + w + v + u + t + s + q + p + o;
}

/* parameters in registers and on the stack, not modified after setjmp */
static double params(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, double x0, double x1,
                     double x2, double x3, double x4, double x5, double x6, double x7, double x8, double x9) {
    if (setjmp(jb) == 0)
        clobber(a);
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10 + x0 + x1 * 2 + x2 * 3 + x3 * 4 +
           x4 * 5 + x5 * 6 + x6 * 7 + x7 * 8 + x8 * 9 + x9 * 10;
}

/* a loop around setjmp: what the loop changes is volatile */
static int loop(int n) {
    volatile int i = 0, acc = 0;
    int base = n * 100;
    int r = setjmp(jb);
    acc += r;
    if (i < n) {
        i++;
        clobber(i);
    }
    return base + acc;
}

static double caller(int n) {
    double keep = n * 0.125;
    long long kept = n * 1000003LL;
    double r = work(n, n * 0.5);
    return r + keep + (double)kept;
}

int main(void) {
    int n;
    double total = 0;
    for (n = 1; n <= 50; n++)
        total += caller(n);
    print_f64(total); /* expect: 1275342018.75 */
    print_f64(work(3, 2.0)); /* expect: 824.5 */
    print_f64(params(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0.5, 1.5, 2.5, 3.5, 4.5, 5.5, 6.5, 7.5, 8.5, 9.5)); /* expect: 742.5 */
    print_i64(loop(5)); /* expect: 520 */
    return 0;
}

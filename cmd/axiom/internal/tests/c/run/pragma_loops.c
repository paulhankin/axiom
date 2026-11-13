/* flags: --c-std=c17 */
/* The loop pragmas (#pragma GCC unroll, ivdep, novector) and the functions'
   options (GCC optimize, push_options, pop_options) change how the code is
   made, not what it computes: loops of trip counts around the unrolling
   factors, known and not known, give their sums. */
#include "host.h"

static int a[300];

#pragma GCC push_options
#pragma GCC optimize ("O3")
static long sum3(int n) {
    long s = 0;
#pragma GCC unroll 3
    for (int i = 0; i < n; i++)
        s += a[i] * (i & 7);
    return s;
}
#pragma GCC optimize 0
static long sum0(int n) {
    long s = 0;
    for (int i = 0; i < n; i++)
        s += a[i] * (i & 7);
    return s;
}
#pragma GCC pop_options

static long known(void) {
    long s = 0;
#pragma GCC unroll 8
    for (int i = 0; i < 61; i++)
        s += a[i] ^ i;
#pragma GCC unroll 100
    for (int i = 0; i < 37; i++)
        s += a[i] * 3;
#pragma GCC unroll 0
    for (int i = 0; i < 4; i++)
        s += a[i] << i;
    int k = 0;
#pragma GCC unroll 4
    while (k < 10) {
        s += k * a[k];
        k++;
    }
#pragma GCC unroll 2
    do {
        s -= a[k];
    } while (--k > 0);
    return s;
}

static void scale(float *d, const float *s, int n) {
#pragma GCC ivdep
    for (int i = 0; i < n; i++)
        d[i] = s[i] * 2.0f;
#pragma GCC novector
    for (int i = 0; i < n; i++)
        d[i] += 1.0f;
}

int main(void) {
    for (int i = 0; i < 300; i++)
        a[i] = (i * 37) % 101 - 50;
    long t = 0;
    for (int n = 0; n < 40; n++)
        t += sum3(n) * (n + 1) - sum0(n);
    print_i64(t);
    print_i64(sum3(299) - sum0(299));
    print_i64(known());
    float x[50], y[50];
    for (int i = 0; i < 50; i++)
        y[i] = (float)i;
    scale(x, y, 37);
    print_f64(x[36] + x[0]);
    return 0;
}
/* expect: -198395 */
/* expect: 0 */
/* expect: -232 */
/* expect: 74 */

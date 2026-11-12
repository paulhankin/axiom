/* Small loops whose trip count is not known, which -O2 runs two iterations
   per test (and the one left over on its own): every count, odd and even,
   none, bounds at the ends of the types, signed and unsigned, a counter
   used after the loop and a value carried from one iteration to the next. */
#include "host.h"
#include <limits.h>

static int a[64];

static long sum(int lo, int n) {
    long s = 0;
    for (int i = lo; i < n; i++) s = s * 3 + a[i & 63];
    return s;
}

static unsigned long long sum_u(unsigned lo, unsigned n) {
    unsigned long long s = 0;
    for (unsigned i = lo; i < n; i++) s = s * 5 + a[i & 63] + i;
    return s;
}

static long long top(long long n) {
    long long s = 0;
    for (long long i = LLONG_MAX - 5; i < n; i++) s += (i & 15) + a[i & 63];
    return s;
}

static unsigned long long top_u(unsigned long long n) {
    unsigned long long s = 0;
    for (unsigned long long i = ULLONG_MAX - 4; i < n; i++) s = s * 7 + (i & 15);
    return s;
}

/* the counter after the loop; a break the unrolled copies keep */
static int find(int n, int x) {
    int i;
    for (i = 0; i < n; i++)
        if (a[i] == x) break;
    return i;
}

/* the carry of a multiply-add, one iteration to the next */
static unsigned long long madd(unsigned *t, const unsigned *x, int n, unsigned m) {
    unsigned long long c = 0;
    for (int j = 0; j < n; j++) {
        unsigned long long p = (unsigned long long)x[j] * m + t[j] + c;
        t[j] = (unsigned)p;
        c = p >> 32;
    }
    return c;
}

/* the body before the test (do-while, goto): the header has effects */
static int hits[8];
static unsigned body_first(int n) {
    int c = 0;
L:
    hits[c & 7] += c + 1;
    if (++c < n) goto L;
    int i = 0;
    do {
        hits[(i * 3) & 7] -= 2;
    } while (++i < n);
    for (int j = 1;; j++) {
        hits[j & 7] += j;
        if (!(j < n)) break;
    }
    unsigned h = 0;
    for (int k = 0; k < 8; k++) h = h * 31u + (unsigned)hits[k];
    return h;
}

static volatile int vk[4] = { 0, 1, 2, 63 };
/* (called through a pointer read from a volatile object: not inlined) */
static unsigned (*volatile body_first_p)(int) = body_first;

int main(void) {
    for (int i = 0; i < 64; i++) a[i] = i * 29 % 31 - 15;
    for (int n = -2; n <= 7; n++) print_i64(sum(vk[0], n));
    print_i64(sum(INT_MIN, INT_MIN + vk[2]));
    print_i64(sum(INT_MIN, INT_MIN));
    print_i64(sum(INT_MAX - 3, INT_MAX));
    print_i64(sum(vk[3], vk[3] + 1));
    for (unsigned n = 0; n <= 5; n++) print_u64(sum_u(vk[0], n));
    print_u64(sum_u(UINT_MAX - 3, UINT_MAX));
    print_u64(sum_u(5, 2));
    print_i64(top(LLONG_MAX));
    print_i64(top(LLONG_MAX - 1));
    print_i64(top(LLONG_MIN));
    print_u64(top_u(ULLONG_MAX));
    print_u64(top_u(0));
    print_i64(find(64, a[vk[3] - 20]));
    print_i64(find(64, 1000));
    print_i64(find(vk[0], 0));
    unsigned t[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    const unsigned x[9] = { 0xffffffffu, 0xfffffffeu, 7, 0x80000000u, 1, 2, 3, 0xffffffffu, 5 };
    for (int n = 0; n <= 9; n++) print_u64(madd(t, x, n, 0xfffffff1u));
    unsigned long long h = 0;
    for (int i = 0; i < 9; i++) h = h * 1000003 + t[i];
    print_u64(h);
    for (int n = 1; n <= 5; n++) print_u64(body_first_p(n));
    return 0;
}
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: -15 */
/* expect: -31 */
/* expect: -81 */
/* expect: -233 */
/* expect: -691 */
/* expect: -2067 */
/* expect: -6197 */
/* expect: -31 */
/* expect: 0 */
/* expect: -153 */
/* expect: 14 */
/* expect: 0 */
/* expect: 18446744073709551601 */
/* expect: 18446744073709551556 */
/* expect: 18446744073709551330 */
/* expect: 18446744073709550199 */
/* expect: 18446744073709544543 */
/* expect: 133143985704 */
/* expect: 0 */
/* expect: 5 */
/* expect: 6 */
/* expect: 0 */
/* expect: 4466 */
/* expect: 0 */
/* expect: 12 */
/* expect: 64 */
/* expect: 0 */
/* expect: 0 */
/* expect: 4294967280 */
/* expect: 4294967280 */
/* expect: 7 */
/* expect: 2147483640 */
/* expect: 1 */
/* expect: 1 */
/* expect: 2 */
/* expect: 4294967280 */
/* expect: 5 */
/* expect: 13259803124641612935 */
/* expect: 3439660642 */
/* expect: 119805314 */
/* expect: 1183575236 */
/* expect: 476151044 */
/* expect: 4063788326 */

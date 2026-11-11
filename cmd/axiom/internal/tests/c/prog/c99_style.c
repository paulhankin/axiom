/* flags: --c-std=c99 */
/* C99 throughout: variable-length arrays (as locals, parameters with
   sizes from earlier parameters, pointers to them, sizeof evaluated at
   run time, in loops that resize them), designated initializers with
   array ranges by index, compound literals as lvalues and as arguments,
   restrict pointers, inline functions, _Bool, long long, mixed
   declarations and statements, declarations in for, // comments,
   __func__, hexadecimal floating constants, flexible array members, and
   the <stdint.h> limits. The workload: matrix products and a Floyd-
   Warshall over VLA matrices, a dynamic-programming edit distance and a
   knapsack with VLA tables, and Pascal's triangle. */
#include "host.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <limits.h>

static inline int64_t min64(int64_t a, int64_t b) { return a < b ? a : b; }
static inline int64_t max64(int64_t a, int64_t b) { return a > b ? a : b; }

// matrix product of VLA parameters
static void matmul(int n, int m, int p, int64_t a[n][m], int64_t b[m][p], int64_t c[restrict n][p]) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < p; j++) {
            int64_t s = 0;
            for (int k = 0; k < m; k++) s += a[i][k] * b[k][j];
            c[i][j] = s;
        }
}

static uint64_t hash_matrix(int n, int m, int64_t (*a)[m]) {
    uint64_t h = 14695981039346656037ull;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++) h = (h ^ (uint64_t)a[i][j]) * 1099511628211ull;
    return h;
}

#define INF (INT64_MAX / 4)

static int64_t floyd(int n, int64_t d[n][n]) {
    for (int k = 0; k < n; k++)
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                if (d[i][k] + d[k][j] < d[i][j]) d[i][j] = d[i][k] + d[k][j];
    int64_t total = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (d[i][j] < INF) total += d[i][j];
    return total;
}

static int edit_distance(const char *restrict a, const char *restrict b) {
    int n = 0, m = 0;
    while (a[n]) n++;
    while (b[m]) m++;
    int dp[n + 1][m + 1];   // a VLA table
    for (int i = 0; i <= n; i++) dp[i][0] = i;
    for (int j = 0; j <= m; j++) dp[0][j] = j;
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= m; j++) {
            int sub = dp[i - 1][j - 1] + (a[i - 1] != b[j - 1]);
            int del = dp[i - 1][j] + 1, ins = dp[i][j - 1] + 1;
            dp[i][j] = sub < del ? (sub < ins ? sub : ins) : (del < ins ? del : ins);
        }
    return dp[n][m];
}

static int64_t knapsack(int n, const int w[static n], const int64_t v[static n], int cap) {
    int64_t best[cap + 1];
    for (int c = 0; c <= cap; c++) best[c] = 0;
    for (int i = 0; i < n; i++)
        for (int c = cap; c >= w[i]; c--) best[c] = max64(best[c], best[c - w[i]] + v[i]);
    return best[cap];
}

struct poly {
    int degree;
    double coef[];   // flexible array member
};

static double horner(const struct poly *p, double x) {
    double r = 0;
    for (int i = p->degree; i >= 0; i--) r = r * x + p->coef[i];
    return r;
}

static int sum_list(int n, const int *xs) {
    int s = 0;
    for (int i = 0; i < n; i++) s += xs[i];
    return s;
}

struct cfg { const char *name; int level; _Bool on; double ratio; int tags[4]; };

static int describe(struct cfg c) {
    return c.level * 100 + c.on * 10 + c.tags[0] + c.tags[3] + (int)(c.ratio * 4);
}

static const char *func_name(void) { return __func__; }

static void add_into(int n, int *restrict dst, const int *restrict a, const int *restrict b) {
    for (int i = 0; i < n; i++) dst[i] = a[i] + b[i];
}

// a designated initializer with gaps, out of order, and overriding
static const int primes_mask[32] = { [2] = 1, [3] = 1, [5] = 1, [7] = 1, [11] = 1, [13] = 1, [17] = 1, [19] = 1,
                                     [23] = 1, [29] = 1, [31] = 1, [4] = 7, [4] = 0 };
static const char *const names[] = { [3] = "three", [1] = "one", "two", [0] = "zero" };

int main(void) {
    // matrices: sizes chosen at run time
    for (int n = 2; n <= 6; n += 2) {
        int m = n + 1, p = n - 1;
        int64_t a[n][m], b[m][p], c[n][p];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++) a[i][j] = (i + 1) * (j + 2) - 3;
        for (int i = 0; i < m; i++)
            for (int j = 0; j < p; j++) b[i][j] = i - 2 * j + 1;
        matmul(n, m, p, a, b, c);
        print_u64(hash_matrix(n, p, c));
        print_i64((int64_t)(sizeof c / sizeof c[0][0]));
        print_i64((int64_t)sizeof(int64_t[n][m]) / 8);
    }
    {
        int n = 7;
        int64_t d[n][n];
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) d[i][j] = i == j ? 0 : (i * 3 + j * 5) % 7 < 3 ? (i + j) % 9 + 1 : INF;
        print_i64(floyd(n, d));
        print_i64(d[n - 1][0]);
        int64_t (*row)[n] = d;   // a pointer to a VLA row
        row++;
        print_i64(row[0][2] < INF ? row[0][2] : -1);
    }
    print_i64(edit_distance("kitten", "sitting"));
    print_i64(edit_distance("intention", "execution"));
    print_i64(edit_distance("", "abc"));
    print_i64(edit_distance("the quick brown fox", "the quack brown fix jumps"));
    {
        int w[] = { 12, 7, 11, 8, 9, 6, 3, 15, 4, 10 };
        int64_t v[] = { 24, 13, 23, 15, 16, 11, 5, 30, 7, 19 };
        print_i64(knapsack(10, w, v, 26));
        print_i64(knapsack(10, w, v, 50));
    }
    {
        // VLAs resized in a loop: each iteration has its own size
        int64_t acc = 0;
        for (int size = 1; size <= 40; size += 3) {
            long long row[size];
            for (int i = 0; i < size; i++) row[i] = (long long)i * i - size;
            for (int i = 1; i < size; i++) row[i] += row[i - 1];
            acc += row[size - 1] + (int64_t)sizeof row;
        }
        print_i64(acc);
    }
    {
        // Pascal's triangle in a VLA of VLA rows (as a 2-D VLA)
        int n = 30;
        unsigned long long t[n][n];
        for (int i = 0; i < n; i++) {
            t[i][0] = t[i][i] = 1;
            for (int j = 1; j < i; j++) t[i][j] = t[i - 1][j - 1] + t[i - 1][j];
        }
        print_u64(t[29][14]);
        print_u64(t[20][10]);
    }
    {
        // compound literals
        print_i64(sum_list(5, (int[]){ 1, 2, 3, 4, 5 }));
        print_i64(sum_list(3, (const int[]){ [2] = 30, [0] = 10 }));
        int *p = (int[3]){ 0 };
        p[1] = 42;
        print_i64(p[0] + p[1] + p[2]);
        (struct cfg){ .level = 1 }.level;   // an unused lvalue
        print_i64(describe((struct cfg){ .name = "x", .level = 3, .on = 1, .ratio = 0.75, .tags = { 4, [3] = 9 } }));
        struct cfg *cp = &(struct cfg){ "y", 2, 0, 1.5, { 1, 2, 3, 4 } };
        cp->on = 5;   // _Bool: any nonzero value is 1
        print_i64(describe(*cp));
        _Bool flags[4] = { 0, 2, -1, 0.5 };
        print_i64(flags[0] + flags[1] * 2 + flags[2] * 4 + flags[3] * 8);
    }
    {
        union { struct poly p; double room[5]; } storage = { { 3 } };
        struct poly *pp = &storage.p;
        double coefs[] = { 1, -2, 0.5, 0.25 };
        for (int i = 0; i <= 3; i++) pp->coef[i] = coefs[i];
        print_f64(horner(pp, 2.0));
        print_f64(horner(pp, -1.5));
    }
    {
        int a[8], b[8], c[8];
        for (int i = 0; i < 8; i++) a[i] = i, b[i] = 10 * i;
        add_into(8, c, a, b);
        print_i64(sum_list(8, c));
        int count = 0;
        for (int i = 0; i < 32; i++) count += primes_mask[i];
        print_i64(count);
        for (int i = 0; i < 4; i++) {
            const char *s = names[i];
            size_t n = 0;
            while (s[n]) n++;
            print_str(s, n);
        }
        const char *f = func_name();
        size_t n = 0;
        while (f[n]) n++;
        print_str(f, n);
    }
    {
        // hexadecimal floats, long long limits
        double x = 0x1.8p3, y = 0x.1p4, z = 0x1p-1074;
        print_f64(x + y);
        print_bool(z > 0 && z / 2 == 0);
        print_f64(0x1.fffffffffffffp1023);
        long long big = LLONG_MAX;
        unsigned long long ubig = ULLONG_MAX;
        print_i64(big);
        print_u64(ubig);
        print_i64(LLONG_MIN);
        print_i64(INT64_MIN + INT32_MAX + UINT16_MAX + INT8_MIN);
        print_u64(UINT64_MAX / UINT32_MAX);
        print_i64(sizeof(intmax_t) == 8 && (int64_t)INTPTR_MAX > 0);
    }
    return 0;
}

/* expect: 592612611215937033 */
/* expect: 2 */
/* expect: 6 */
/* expect: 13683363062078570573 */
/* expect: 12 */
/* expect: 20 */
/* expect: 7346919279255424061 */
/* expect: 30 */
/* expect: 42 */
/* expect: 352 */
/* expect: 9 */
/* expect: 13 */
/* expect: 3 */
/* expect: 5 */
/* expect: 3 */
/* expect: 8 */
/* expect: 53 */
/* expect: 99 */
/* expect: 72625 */
/* expect: 77558760 */
/* expect: 184756 */
/* expect: 15 */
/* expect: 40 */
/* expect: 42 */
/* expect: 326 */
/* expect: 221 */
/* expect: 14 */
/* expect: 1 */
/* expect: 4.28125 */
/* expect: 308 */
/* expect: 11 */
/* expect: zero */
/* expect: one */
/* expect: two */
/* expect: three */
/* expect: func_name */
/* expect: 13 */
/* expect: true */
/* expect: 1.7976931348623157e+308 */
/* expect: 9223372036854775807 */
/* expect: 18446744073709551615 */
/* expect: -9223372036854775808 */
/* expect: -9223372034707226754 */
/* expect: 4294967297 */
/* expect: 1 */

/* flags: --c-std=gnu17 */
/* Nested functions (a GNU dialect) and the variable length arrays and
   variably modified types of their enclosing functions: an array used
   (read, written, its size), allocated again in a loop, two-dimensional, from
   a function nested twice, through a trampoline; a pointer to an array of
   variable length, a typedef, typeof, and a nested function's parameter whose
   type's length is the enclosing function's object. */
#include "host.h"

static int run(int (*f)(int), int x) { return f(x); }

static int test(int n) {
    int v[n];
    for (int i = 0; i < n; i++)
        v[i] = i * i;
    int get(int i) { return v[i]; }
    void put(int i, int x) { v[i] = x; }
    int size(void) { return (int)(sizeof v / sizeof v[0]); }
    put(1, 100);
    int r = get(1) + get(n - 1) + size() * 1000;

    /* two dimensions: the row's length is the enclosing function's */
    int m = n + 1;
    int g[n][m];
    for (int i = 0; i < n; i++)
        for (int j = 0; j < m; j++)
            g[i][j] = i * 10 + j;
    int cell(int i, int j) { return g[i][j] + (int)sizeof g[0]; }
    r += cell(2, 3) * 10000;

    /* nested twice, and through a trampoline */
    int outer(int k) {
        int inner(int i) { return v[i] + k + (int)(sizeof g / sizeof(int)); }
        return run(inner, 2);
    }
    r += outer(7) * 1000000;
    return r;
}

static int loops(void) {
    int total = 0;
    for (int n = 1; n <= 4; n++) {
        int a[n];
        for (int i = 0; i < n; i++)
            a[i] = n;
        int sum(void) {
            int s = 0;
            for (unsigned i = 0; i < sizeof a / sizeof a[0]; i++)
                s += a[i];
            return s;
        }
        total = total * 100 + sum();
    }
    return total;
}

static int types(int n) {
    int buf[3][5] = { { 0 } };
    buf[2][4] = 9;
    int (*p)[n] = buf;  /* n is 5 */
    typedef int row[n];
    int via_ptr(void) { return p[2][4] + (int)(sizeof *p / sizeof(int)); }
    int via_typedef(void) {
        row r;
        typeof(p) q = p;
        return (int)sizeof r + (int)sizeof *q;
    }
    int param(int a[][n]) { return a[2][4] * 100 + (int)sizeof a[0]; }
    return via_ptr() * 10000 + via_typedef() * 100 + param(buf) / 100;
}

int main(void) {
    print_i64(test(4));
    print_i64(loops());
    print_i64(types(5));
    return 0;
}
/* expect: 31434109 */
/* expect: 1040916 */
/* expect: 144009 */

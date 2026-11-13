/* flags: --c-std=gnu17 */
/* Nested functions (a GNU dialect): the enclosing functions' objects
   through the chain of frame records, calls through pointers
   (trampolines), recursion of the enclosing function (each call's own
   frame), nested functions nested again, structure arguments and results,
   a forward declaration with auto (a jump to an enclosing function's label is not
   supported: err_gnu_nested_goto.c). */
#include "host.h"

typedef int (*fn)(int);
static int apply(fn f, int x) { return f(x); }
static void sort(int *v, int n, int (*less)(int, int)) {
    for (int i = 1; i < n; i++)
        for (int j = i; j > 0 && less(v[j], v[j - 1]); j--) {
            int t = v[j];
            v[j] = v[j - 1];
            v[j - 1] = t;
        }
}

struct S { int a[5]; double d; };

static int outer(int base) {
    int count = 0;
    struct S s = {{1, 2, 3, 4, 5}, 2.5};
    int add(int y) { count++; return base + y + s.a[4]; }
    int twice(int y) {
        int inner(int z) { count += 10; return add(z) * 2; }
        return inner(y) + apply(inner, y);
    }
    int r = add(1) + apply(add, 2);
    r += twice(3);
    print_i64(count);
    return r;
}

static int level(int n) {
    int k = n * 10;
    int f(int x) { return x + k; }
    if (n > 0)
        return level(n - 1) + apply(f, 1);
    return apply(f, 1);
}

static long many(int i) {
    long acc = i;
    long g(int x) {
        long h(void) { return acc * 2 + x; }
        acc += x;
        return h();
    }
    long (*pg)(int) = g;
    if (i % 3 == 0)
        return pg(i);
    return g(i);
}

int main(void) {
    int dir = -1;
    int less(int a, int b) { return dir * (a - b) < 0; }
    int v[6] = {3, 1, 4, 1, 5, 9};
    sort(v, 6, less);
    print_i64(v[0] * 100000 + v[1] * 10000 + v[2] * 1000 + v[3] * 100 + v[4] * 10 + v[5]);
/* expect: 954311 */
    dir = 1;
    sort(v, 6, less);
    print_i64(v[0] * 100000 + v[1] * 10000 + v[2] * 1000 + v[3] * 100 + v[4] * 10 + v[5]);
/* expect: 113459 */
    print_i64(outer(100));
/* expect: 24 */
/* expect: 645 */
    print_i64(level(50));
/* expect: 12801 */
    long s = 0;
    for (int i = 0; i < 2000; i++)
        s += many(i);
    print_i64(s);
/* expect: 9995000 */
    struct S big(struct S x, double k) { x.d *= k; x.a[0] += dir; return x; }
    struct S t = {{7}, 1.5};
    struct S (*pb)(struct S, double) = big;
    t = pb(t, 4.0);
    print_f64(t.d + t.a[0]);
/* expect: 14 */
    auto int later(int);
    int w = later(5);
    int later(int z) { return z * 3 + dir; }
    print_i64(w);
/* expect: 16 */
    return 0;
}

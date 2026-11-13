/* flags: --c-std=gnu17 */
/* A designator range whose initializer has effects: the initializer is
   evaluated once, and every element it designates gets its value (the GNU
   dialects' rule), whatever the element's type, in nested lists and in
   compound literals, and when a later designator overrides an element. */
#include "host.h"

static int calls;
static int next(void) { return ++calls; }
struct pt { int x, y; };
static struct pt mk(int x) { calls++; struct pt p = { x, x * 2 }; return p; }

int main(void) {
    int a[6] = { [0 ... 4] = next(), [5] = 9 };
    print_i64(calls);
    print_i64(a[0] * 1000 + a[2] * 100 + a[4] * 10 + a[5]);

    calls = 0;
    struct pt ps[4] = { [1 ... 3] = { next(), next() } };
    print_i64(calls);
    print_i64(ps[0].x * 1000 + ps[1].x * 100 + ps[3].x * 10 + ps[3].y);

    calls = 0;
    struct pt qs[3] = { [0 ... 2] = mk(4) };
    print_i64(calls);
    print_i64(qs[0].y + qs[2].x * 10);

    calls = 0;
    int g[2][3] = { [0 ... 1][0 ... 2] = next() };
    print_i64(calls);
    print_i64(g[0][0] + g[1][2]);

    /* the value converted to each element's type */
    int k = 300;
    unsigned char c[3] = { [0 ... 2] = k++ };
    print_i64(k);
    print_i64(c[0] + c[2]);

    /* overridden: evaluated once all the same */
    calls = 0;
    int o[3] = { [0 ... 2] = next(), [0] = 7, [1] = 8, [2] = 9 };
    print_i64(calls);
    print_i64(o[0] + o[1] + o[2]);

    /* in a compound literal, in a loop */
    calls = 0;
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        int *p = (int[4]){ [0 ... 3] = next() + i };
        sum += p[0] + p[3];
    }
    print_i64(calls);
    print_i64(sum);

    /* volatile reads count once */
    volatile int v = 5;
    int w[3] = { [0 ... 2] = v };
    print_i64(w[0] + w[1] + w[2]);
    return 0;
}
/* expect: 1 */
/* expect: 1119 */
/* expect: 2 */
/* expect: 112 */
/* expect: 1 */
/* expect: 48 */
/* expect: 1 */
/* expect: 2 */
/* expect: 301 */
/* expect: 88 */
/* expect: 1 */
/* expect: 24 */
/* expect: 3 */
/* expect: 18 */
/* expect: 15 */

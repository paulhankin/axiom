/* flags: --c-std=c23 */
/* Structures aligned to more than 16 bytes as arguments and results: the
   parameters' objects have their alignment, their values arrive whole. */
#include "host.h"

struct A { alignas(32) int x; int y; };
struct B { alignas(64) char c[3]; };

static int take(int a, struct A s, int b, struct B t, struct A u) {
    return a + s.x * 10 + s.y * 100 + b * 1000 + t.c[2] * 10000 + u.x * 100000 +
           (int)((unsigned long)&s % 32 == 0) * 1000000 + (int)((unsigned long)&t % 64 == 0) * 2000000;
}

static struct A give(int v) {
    struct A r = { v, v + 1 };
    return r;
}

int main(void) {
    struct A s = { 1, 2 };
    struct B t = { { 0, 0, 3 } };
    struct A u = give(4);
    print_i64(take(5, s, 6, t, u));
    print_i64(u.y);
    struct A arr[2];
    print_i64((unsigned long)&arr[1] % 32);
    print_i64(sizeof(struct A));
    return 0;
}
/* expect: 3436215 */
/* expect: 5 */
/* expect: 0 */
/* expect: 32 */

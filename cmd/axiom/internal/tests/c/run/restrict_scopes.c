/* restrict on block-scope pointers, on parameters of inlined functions and
   on structure members (6.7.3.1), in the cases where the program may
   reach one object through pointers that are not based on one another:
   when none of them writes it, after the block (a copy of the pointer), in
   another execution of the block (a loop's next iteration, another call of
   an inlined function), in a block with a label, and through an inner
   block's restricted pointer based on an outer one's. The optimizer may
   take accesses through two restricted pointers of one block for apart only
   where the program could not tell. */
#include "host.h"

/* both read the same object */
static int reads(int *a) {
    int *restrict p = a;
    int *restrict q = a;
    return *p + *q;
}

/* the pointer copied out of its block, used after it with another */
static int copied(int *a) {
    int *c;
    {
        int *restrict p = a;
        *p = 3;
        c = p;
    }
    *a = 4;
    return *c;
}

/* an inner block's restricted pointer based on the outer one's */
static int nested(int *a) {
    int *restrict p = a;
    p[1] = 1;
    {
        int *restrict q = p + 1;
        q[0] = 5;
    }
    return p[1];
}

/* a block executed again: its pointers overlap the last execution's */
static int iterations(int *a, int n) {
    for (int i = 0; i < n; i++) {
        int *restrict p = a + i;
        int *restrict q = a + i + 1;
        *q = *p + 1;
    }
    return a[n];
}

/* an inlined function's restrict parameters: each call is a scope */
static inline void inc(int *restrict d, const int *restrict s) { *d = *s + 1; }
static int calls(int *a, int n) {
    for (int i = 0; i < n; i++)
        inc(&a[i + 1], &a[i]);
    return a[n];
}
static inline int sum2(const int *restrict p, const int *restrict q) { return *p + *q; }

/* a block with a label (no scope: a jump could enter it) */
static int labelled(int *a, int k) {
    if (k)
        goto in;
    {
        int *restrict p;
        p = a;
        *p = 1;
    in:
        p = a + 1;
        *p = 2;
        return p[0];
    }
}

/* restrict-qualified members on one object that none of them writes, and
   two structure objects whose members are apart within each */
struct two {
    int *restrict x;
    int *restrict y;
};
static int members(int *a, int *b) {
    struct two s = { a, a };
    int r = s.x[0] + s.y[0];
    struct two t = { a, b };
    t.x[0] = 7;
    t.y[0] = 8;
    return r * 100 + t.x[0] * 10 + t.y[0];
}

int main(void) {
    int a[16] = { 6 };
    print_i64(reads(a));
    print_i64(copied(a));
    print_i64(nested(a));
    a[0] = 10;
    print_i64(iterations(a, 8));
    a[0] = 20;
    print_i64(calls(a, 8));
    print_i64(sum2(a, a));
    print_i64(labelled(a, 0) + labelled(a, 1));
    int x = 3, y = 0;
    print_i64(members(&x, &y));
    return 0;
}
/* expect: 12 */
/* expect: 4 */
/* expect: 5 */
/* expect: 18 */
/* expect: 28 */
/* expect: 40 */
/* expect: 4 */
/* expect: 678 */

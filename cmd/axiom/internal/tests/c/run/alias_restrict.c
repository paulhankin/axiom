/* flags: --c-std=c11 */
/* What the optimizer may and may not take as apart (gvn.ax, ir.ax:
   restrict_apart, min_offset): a restrict-qualified parameter's accesses
   and another parameter's are apart, but two parameters without restrict
   may be the same object, and so may a pointer loaded from memory that
   holds the restrict one's value (it is based on it); an array indexed from
   0 by a counter that only grows is apart from a field before it, but not
   when the same memory is reached from the start of the structure. */
#include "host.h"
struct big { int n; int d[8]; };
static struct big g = { 3, { 1, 2, 3, 4, 5, 6, 7, 8 } };
static int *volatile hidden;

static int with_restrict(int *restrict a, const int *b, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        a[i] = *b + i;  /* *b may stay in a register */
        s += *b;
    }
    return s;
}
static int without(int *a, const int *b, int n) {
    int s = 0;
    for (int i = 0; i < n; i++) {
        a[i] = *b + i;  /* a and b may be one object: *b is read again */
        s += *b;
    }
    return s;
}
static int based(int *restrict a, int n) {
    int *q;
    int s = 0;
    hidden = a;
    q = hidden;  /* q's value is a's: based on it */
    for (int i = 0; i < n; i++) {
        q[0] = i + 1;
        s += a[0];
    }
    return s;
}
static int fields(int k) {
    int s = 0;
    for (int i = 0; i < k; i++) {
        g.d[i] = g.n + i;  /* d[i], i >= 0, is apart from n */
        s += g.n;
    }
    return s;
}
static int from_start(int k) {
    int *p = (int *)&g;
    int s = 0;
    for (int i = 0; i < k; i++) {
        p[i] = g.n + 10;  /* p[0] is n */
        s += g.n;
    }
    return s;
}
int main(void) {
    int x[8] = { 0 }, y = 5;
    print_i64(with_restrict(x, &y, 4) * 100 + x[3]);
    print_i64(without(x, x + 1, 4) * 100 + x[3]);
    print_i64(based(x, 4));
    print_i64(fields(5) * 100 + g.d[4]);
    print_i64(from_start(3) * 100 + g.n);
    return 0;
}
/* expect: 2008 */
/* expect: 2710 */
/* expect: 10 */
/* expect: 1507 */
/* expect: 3913 */

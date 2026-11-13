/* Jump threading through several blocks (thread.ax, thread_region_of): the
   edge where a test held decides the same test, or one it implies, a
   diamond or two further down, and the blocks between are copied for it.
   fix's second tests of l and r (an AVL tree's heights: 3 tests of a
   pointer, one of r's with each way of l's; 5 without the threading);
   implied's `x > 5` after `x > 10` and flagged's `f`, a phi of
   constants, after the test of q in between (whose test is copied: 2 of
   them, and 1 test of x in flagged). */
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \btest\s+%r\w+,%r\w+ | 7
// lower-selfhost: --target=native --cpu=x86-64 -O2 --c-freestanding | \btest\s+%edi,%edi | 1
// lower: --target=aarch64 -O2 --c-freestanding | \bcmp\s+x\d+, #0x0 | 7
struct node { struct node *l, *r; int h; };
static int ht(const struct node *t) { return t ? t->h : 0; }
int fix(struct node *t) {
    int a = ht(t->l), b = ht(t->r);
    t->h = (a > b ? a : b) + 1;
    return ht(t->l) - ht(t->r);
}
int implied(int x, int *p, int *q) {
    int a = x > 10 ? *p : 0;
    int b = q ? *q : 0;
    p[1] = a + b;
    if (x > 5) return a;
    return b;
}
int flagged(int x, int *p, int *q) {
    int f = 0, a = 0;
    if (x) { f = 1; a = *p; }
    int b = q ? *q : 0;
    p[1] = b;
    if (f) return a + b;
    return b - 1;
}

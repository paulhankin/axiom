/* flags: -O2 */
/* Jump threading through several blocks (thread.ax, thread_region_of): a
   branch whose condition the edge into a block further up decides, with a
   diamond or two in between, is threaded by copying the blocks between.
   Null tests repeated two diamonds below (an AVL tree's heights), a
   comparison implied by an earlier one, a phi of constants, the values the
   copied blocks compute used after them, a volatile access in the copied
   blocks (done once on every path), and the boundaries of the comparisons. */
#include "host.h"

typedef struct avl {
    struct avl *l, *r;
    unsigned key;
    int height;
} avl;

static int ht(const avl *t) { return t ? t->height : 0; }
static void fix(avl *t) {
    int a = ht(t->l), b = ht(t->r);
    t->height = (a > b ? a : b) + 1;
}
static avl *rot_right(avl *t) {
    avl *l = t->l;
    t->l = l->r;
    l->r = t;
    fix(t);
    fix(l);
    return l;
}
static avl *rot_left(avl *t) {
    avl *r = t->r;
    t->r = r->l;
    r->l = t;
    fix(t);
    fix(r);
    return r;
}
static int rotations;
static avl *balance(avl *t) {
    fix(t);
    int bf = ht(t->l) - ht(t->r);
    if (bf > 1) {
        if (ht(t->l->l) < ht(t->l->r)) { t->l = rot_left(t->l); rotations++; }
        rotations++;
        return rot_right(t);
    }
    if (bf < -1) {
        if (ht(t->r->r) < ht(t->r->l)) { t->r = rot_right(t->r); rotations++; }
        rotations++;
        return rot_left(t);
    }
    return t;
}
static avl pool[600];
static int used;
static avl *insert(avl *t, unsigned key) {
    if (!t) {
        avl *n = &pool[used++];
        n->l = n->r = 0;
        n->key = key;
        n->height = 1;
        return n;
    }
    if (key < t->key) t->l = insert(t->l, key);
    else if (key > t->key) t->r = insert(t->r, key);
    else return t;
    return balance(t);
}
static int check(const avl *t, int *count) {
    if (!t) return 0;
    (*count)++;
    int bad = 0, bf = ht(t->l) - ht(t->r);
    if (bf < -1 || bf > 1) bad++;
    if (t->height != (ht(t->l) > ht(t->r) ? ht(t->l) : ht(t->r)) + 1) bad++;
    return bad + check(t->l, count) + check(t->r, count);
}

/* `x > 5` is known on the edge where `x > 10` held */
static int implied(int x, int *p, int *q) {
    int a = x > 10 ? *p : 0;
    int b = q ? *q : 0;
    p[1] = a + b;
    if (x > 5) return a;
    return b;
}
/* unsigned: `u == 15` is known false where `u < 10` held, and true where
   `u == 15` held */
static int unsigned_facts(unsigned u, int *p, int *q) {
    int a = 0;
    if (u < 10) a = *p;
    else if (u == 15) a = p[2];
    int b = q ? *q : 1;
    p[3] = a - b;
    if (u == 15) return a * 2;
    if (u < 10) return a + 100;
    return b;
}
/* f, a phi of constants, decides the last branch on both edges */
static volatile int touched;
static int flagged(int x, int *p, int *q, int *out) {
    int f = 0, a = 0;
    if (x) { f = 1; a = *p; }
    int b = q ? *q : 7;
    touched++;
    int w = b * 3 + a;  /* computed in the copied blocks, used after them */
    if (f) out[0] = w;
    else out[1] = w;
    return w + b + f;
}
/* the same pointer tested three times, a diamond each */
static long thrice(long *p, long *q) {
    long a = p ? *p : -1;
    long b = q ? *q : -2;
    long c = p ? p[1] : -3;
    long d = q ? q[1] : -4;
    long e = p ? p[2] : -5;
    return a * 10000 + b * 1000 + c * 100 + d * 10 + e;
}

/* (called through pointers: not inlined into main) */
static int (*volatile implied_p)(int, int *, int *) = implied;
static int (*volatile unsigned_p)(unsigned, int *, int *) = unsigned_facts;
static int (*volatile flagged_p)(int, int *, int *, int *) = flagged;
static long (*volatile thrice_p)(long *, long *) = thrice;

static int vals[] = { -2147483647 - 1, -1, 0, 5, 6, 10, 11, 2147483647 };
static unsigned uvals[] = { 0, 9, 10, 14, 15, 16, 0xffffffffu };

int main(void) {
    avl *t = 0;
    unsigned s = 12345;
    for (int i = 0; i < 500; i++) {
        s = s * 1103515245u + 12345u;
        t = insert(t, (s >> 8) % 1000);
    }
    int count = 0;
    print_i64(check(t, &count));
    print_i64(count);
    print_i64(ht(t));
    print_i64(rotations);

    int p[4] = { 3, 0, 4, 0 }, q[2] = { 20, 0 };
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum = sum * 7 + implied_p(vals[i], p, q) + p[1];
        sum = sum * 7 + implied_p(vals[i], p, 0) + p[1];
    }
    print_i64(sum);
    sum = 0;
    for (int i = 0; i < 7; i++) {
        sum = sum * 5 + unsigned_p(uvals[i], p, q) + p[3];
        sum = sum * 5 + unsigned_p(uvals[i], p, 0) + p[3];
    }
    print_i64(sum);
    int out[2] = { 0, 0 };
    sum = 0;
    for (int x = -1; x <= 1; x++) {
        sum = sum * 3 + flagged_p(x, p, q, out) + out[0] - out[1];
        sum = sum * 3 + flagged_p(x, p, 0, out) + out[0] - out[1];
    }
    print_i64(sum);
    print_i64(touched);
    long lp[3] = { 1, 2, 3 }, lq[2] = { 4, 5 };
    print_i64(thrice_p(lp, lq));
    print_i64(thrice_p(lp, 0));
    print_i64(thrice_p(0, lq));
    print_i64(thrice_p(0, 0));
    return 0;
}
/* expect: 0 */
/* expect: 395 */
/* expect: 10 */
/* expect: 281 */
/* expect: 193858744844400 */
/* expect: 135839825625 */
/* expect: 42137 */
/* expect: 6 */
/* expect: 14253 */
/* expect: 8163 */
/* expect: -6255 */
/* expect: -12345 */

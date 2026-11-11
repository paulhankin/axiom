/* Structures and unions: members, assignment, nesting, self-reference,
   by-value parameters and results (small and large), unions' sharing. */
#include "host.h"

struct pt { int x, y; };
struct rect { struct pt a, b; char tag; };
struct big { long long v[6]; int n; };
struct node { int val; struct node *next; };
union num { int i; unsigned char b[4]; float f; };
struct mixed { char c; double d; short s; int *p; };

static struct pt mk(int x, int y) {
    struct pt p;
    p.x = x;
    p.y = y;
    return p;
}

static struct pt add(struct pt a, struct pt b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

static struct big fill(int n) {
    struct big b;
    int i;
    for (i = 0; i < 6; i++) b.v[i] = (long long)n * i;
    b.n = n;
    return b;
}

static long long total(struct big b) {
    long long t = b.n;
    int i;
    for (i = 0; i < 6; i++) t += b.v[i];
    b.n = 0;
    return t;
}

static int area(const struct rect *r) {
    return (r->b.x - r->a.x) * (r->b.y - r->a.y);
}

int main(void) {
    struct pt p = mk(3, 4), q;
    struct rect r;
    struct big b, c;
    struct node n3 = { 3, 0 }, n2 = { 2, &n3 }, n1 = { 1, &n2 };
    struct node *it;
    union num u;
    struct mixed mx;
    int s = 0;
    q = p;
    q.x = 10;
    print_i64(p.x);
    print_i64(q.x + q.y);
    p = add(p, q);
    print_i64(p.x);
    print_i64(p.y);
    r.a = mk(1, 1);
    r.b = add(r.a, mk(4, 5));
    r.tag = 'R';
    print_i64(area(&r));
    print_i64(r.tag);
    b = fill(7);
    c = b;
    b.v[0] = 1000;
    print_i64(total(c));
    print_i64(total(b));
    print_i64(b.n);
    for (it = &n1; it; it = it->next) s = s * 10 + it->val;
    print_i64(s);
    print_i64(n1.next->next->val);
    u.i = 0x01020304;
    print_i64(u.b[0]);
    print_i64(u.b[3]);
    u.f = 1.0f;
    print_u64((unsigned)u.i);
    print_i64(sizeof u);
    print_i64(sizeof(struct rect));
    print_i64(sizeof(struct big));
    mx.c = 'a';
    mx.d = 2.5;
    mx.s = -3;
    mx.p = &s;
    *mx.p = 42;
    print_i64(s);
    print_f64(mx.d * mx.s);
    print_i64(mk(5, 6).y);
    print_i64(fill(2).v[5]);
    {
        struct pt arr[3];
        arr[0] = mk(1, 2);
        arr[1] = arr[0];
        arr[2] = add(arr[0], arr[1]);
        print_i64(arr[2].x * 10 + arr[2].y);
    }
    {
        struct pt t1 = { 1, 2 }, t2 = { 3, 4 };
        struct pt *pp = s > 0 ? &t1 : &t2;
        struct pt t3 = s > 0 ? t2 : t1;
        print_i64(pp->y);
        print_i64(t3.x);
        t1 = t2 = mk(8, 9);
        print_i64(t1.x + t2.y);
    }
    return 0;
}

/* expect: 3 */
/* expect: 14 */
/* expect: 13 */
/* expect: 8 */
/* expect: 20 */
/* expect: 82 */
/* expect: 112 */
/* expect: 1112 */
/* expect: 7 */
/* expect: 123 */
/* expect: 3 */
/* expect: 4 */
/* expect: 1 */
/* expect: 1065353216 */
/* expect: 4 */
/* expect: 20 */
/* expect: 56 */
/* expect: 42 */
/* expect: -7.5 */
/* expect: 6 */
/* expect: 10 */
/* expect: 24 */
/* expect: 2 */
/* expect: 3 */
/* expect: 17 */

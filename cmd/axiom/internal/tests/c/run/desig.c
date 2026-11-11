/* Designated initializers (C99 6.7.9): .member and [index], nested
   designator lists, the walk continuing after a designation (brace
   elision included), later initializers overriding earlier ones, the
   length of an array from its highest index, anonymous members, unions,
   bit-fields; in static and automatic objects. */
#include "host.h"
struct P { int x, y; };
struct L { struct P a, b; int tag; };
struct N { int k; struct { int m, n; }; union { short s; long long w; }; int last; };
struct F { unsigned a:4, b:4; int c; };
union V { char c[4]; int i; };
static int a1[] = { [3] = 30, 1, [1] = 10, 11 };
static struct L l1 = { .b = { 3, 4 }, .a.y = 2, 9 };
static struct L l2 = { .b.x = 5, 6, 7, .a = { 1 } };
static struct N n1 = { .m = 1, 2, .w = 99, 7 };
static struct N n2 = { 1, 2, 3, 4, 5 };
static struct F f1 = { .b = 9, .a = 3, .c = -1 };
static struct P ps[] = { [2].y = 5, [0] = { .y = 1, .x = 2 } };
static int m2[3][3] = { [1] = { 1, 2 }, [2][2] = 9, [0][1] = 4 };
static char str[10] = { [2] = 'x', 'y' };
static struct P over[2] = { [0] = { 1, 2 }, [0].y = 3, [1] = { 4, 5 }, [1] = { .x = 6 } };
static union V uv = { .i = 0x01020304 };
static union V uv2 = { .c = { 1, 2 }, .c[3] = 7 };
int main(void) {
    int i;
    for (i = 0; i < (int)(sizeof a1 / sizeof a1[0]); i++) print_i64(a1[i]);
    print_i64(sizeof a1);
    print_i64(l1.a.x); print_i64(l1.a.y); print_i64(l1.b.x); print_i64(l1.b.y); print_i64(l1.tag);
    print_i64(l2.a.x); print_i64(l2.a.y); print_i64(l2.b.x); print_i64(l2.b.y); print_i64(l2.tag);
    print_i64(n1.k); print_i64(n1.m); print_i64(n1.n); print_i64(n1.w); print_i64(n1.last);
    print_i64(n2.k); print_i64(n2.m); print_i64(n2.n); print_i64(n2.s); print_i64(n2.last);
    print_i64(f1.a); print_i64(f1.b); print_i64(f1.c);
    print_i64(sizeof ps / sizeof ps[0]); print_i64(ps[0].x); print_i64(ps[0].y); print_i64(ps[2].y); print_i64(ps[1].x);
    for (i = 0; i < 9; i++) print_i64(m2[i / 3][i % 3]);
    print_i64(str[2] + str[3] + str[0] + str[9]);
    print_i64(over[0].x); print_i64(over[0].y); print_i64(over[1].x); print_i64(over[1].y);
    print_i64(uv.i); print_i64(uv2.c[0]); print_i64(uv2.c[1]); print_i64(uv2.c[3]);
    {
        int k = 5;
        int la[] = { [4] = k, [1] = k * 2, k + 1 };
        struct L ll = { .b = { k, k }, .a.y = k + 2, k * 3 };
        struct N nn = { .m = k, k + 1, .w = k * 100, k * 7 };
        struct F ff = { .b = k + 10, .a = k };
        struct P over2[2] = { [0] = { k, 2 }, [0].y = 3, [1] = { 4, 5 }, [1] = { .x = k } };
        for (i = 0; i < 5; i++) print_i64(la[i]);
        print_i64(ll.a.x + ll.a.y * 10 + ll.b.x * 100 + ll.b.y * 1000 + ll.tag * 10000);
        print_i64(nn.k); print_i64(nn.m); print_i64(nn.n); print_i64(nn.w); print_i64(nn.last);
        print_i64(ff.a); print_i64(ff.b); print_i64(ff.c);
        print_i64(over2[0].x); print_i64(over2[0].y); print_i64(over2[1].x); print_i64(over2[1].y);
    }
    return 0;
}
/* expect: 0 */
/* expect: 10 */
/* expect: 11 */
/* expect: 30 */
/* expect: 1 */
/* expect: 20 */
/* expect: 0 */
/* expect: 2 */
/* expect: 9 */
/* expect: 4 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 5 */
/* expect: 6 */
/* expect: 7 */
/* expect: 0 */
/* expect: 1 */
/* expect: 2 */
/* expect: 99 */
/* expect: 7 */
/* expect: 1 */
/* expect: 2 */
/* expect: 3 */
/* expect: 4 */
/* expect: 5 */
/* expect: 3 */
/* expect: 9 */
/* expect: -1 */
/* expect: 3 */
/* expect: 2 */
/* expect: 1 */
/* expect: 5 */
/* expect: 0 */
/* expect: 0 */
/* expect: 4 */
/* expect: 0 */
/* expect: 1 */
/* expect: 2 */
/* expect: 0 */
/* expect: 0 */
/* expect: 0 */
/* expect: 9 */
/* expect: 241 */
/* expect: 1 */
/* expect: 3 */
/* expect: 6 */
/* expect: 0 */
/* expect: 16909060 */
/* expect: 1 */
/* expect: 2 */
/* expect: 7 */
/* expect: 0 */
/* expect: 10 */
/* expect: 6 */
/* expect: 0 */
/* expect: 5 */
/* expect: 6570 */
/* expect: 0 */
/* expect: 5 */
/* expect: 6 */
/* expect: 500 */
/* expect: 35 */
/* expect: 5 */
/* expect: 15 */
/* expect: 0 */
/* expect: 5 */
/* expect: 3 */
/* expect: 5 */
/* expect: 0 */

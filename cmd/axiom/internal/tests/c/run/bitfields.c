/* Bit-fields: their layout (packed into units of their type, a new unit
   when one would straddle, zero-width and unnamed ones), loads and stores
   with sign extension, compound assignment and ++/--, the value of an
   assignment, the promotion of narrow ones to int, _Bool and long long
   bit-fields, unions, static and automatic initializers. */
#include "host.h"

struct A { unsigned a : 3, b : 5; int c : 4; unsigned d : 20; _Bool e : 1; char f; };
struct B { char x; int y : 7; long long z : 40; short w : 9; };
struct C { int : 0; unsigned char q : 2; int : 3; unsigned r : 1; };
struct D { char c; int : 0; char d; };
struct E { char a; int : 3; char b; };
struct G { char a; int b : 30; };
union U { int a : 5; unsigned b : 12; };
struct H { unsigned lo : 4, hi : 4; } ;

static struct A sa = { 5, 17, -3, 0xABCDE, 1, 'f' };
struct A garr[2] = { { 1, 2, 3, 4, 0, 5 }, { 7, 31, -8, 1048575, 1, 6 } };
static struct H hs[3] = { { 1, 2 }, { 15, 15 }, { 0, 9 } };

static unsigned get_d(struct A *p) { return p->d; }

int main(void) {
    struct A a = { 5, 17, -3, 0xABCDE, 1, 'f' };
    struct B b;
    union U u;
    struct C c = { 3, 1 };
    int k;
    unsigned x;
    print_i64(sizeof(struct A));
    print_i64(sizeof(struct B));
    print_i64(sizeof(struct C));
    print_i64(sizeof(struct D));
    print_i64(sizeof(struct E));
    print_i64(sizeof(struct G));
    print_i64(sizeof(union U));
    print_i64(_Alignof(struct D));
    print_i64(a.a + a.b * 10 + a.c * 1000);
    print_i64(a.d);
    print_i64(a.e + a.f);
    print_i64(sa.a + sa.b * 10 + sa.c * 1000);
    print_i64(sa.d + sa.e + sa.f);
    a.a = 9;
    print_i64(a.a);
    a.c = 7;
    a.c++;
    print_i64(a.c);
    a.b += 20;
    print_i64(a.b);
    a.d = -1;
    print_i64(a.d);
    print_i64(get_d(&a));
    print_i64(a.a - 2);
    print_i64(garr[1].c + garr[1].b + garr[0].d);
    b.x = 1;
    b.y = -64;
    b.z = -5;
    b.w = 255;
    print_i64(b.y);
    print_i64(b.z);
    print_i64(b.w);
    print_i64(b.x);
    b.z = 549755813887LL;
    print_i64(b.z);
    b.z += 1;
    print_i64(b.z);
    u.b = 0xFFF;
    print_i64(u.a);
    print_i64(u.b);
    k = (a.e = 5);
    print_i64(k);
    x = a.a = 15;
    print_i64(x);
    print_i64(a.c--);
    print_i64(a.c);
    print_i64(--a.b);
    print_i64(c.q + c.r * 10);
    a.b = 0;
    a.b -= 1;
    print_i64(a.b);
    a.c = 5;
    a.c /= -2;
    print_i64(a.c);
    a.a = 7;
    a.a <<= 1;
    print_i64(a.a);
    print_i64(-a.a < 0);
    print_i64(hs[0].lo + hs[0].hi * 16 + hs[1].lo + hs[1].hi + hs[2].hi);
    hs[2].lo = hs[1].hi + 1;
    print_i64(hs[2].lo + hs[2].hi);
    return 0;
}
/* expect: 8 */
/* expect: 8 */
/* expect: 4 */
/* expect: 5 */
/* expect: 3 */
/* expect: 8 */
/* expect: 4 */
/* expect: 1 */
/* expect: -2825 */
/* expect: 703710 */
/* expect: 103 */
/* expect: -2825 */
/* expect: 703813 */
/* expect: 1 */
/* expect: -8 */
/* expect: 5 */
/* expect: 1048575 */
/* expect: 1048575 */
/* expect: -1 */
/* expect: 27 */
/* expect: -64 */
/* expect: -5 */
/* expect: 255 */
/* expect: 1 */
/* expect: 549755813887 */
/* expect: -549755813888 */
/* expect: -1 */
/* expect: 4095 */
/* expect: 1 */
/* expect: 7 */
/* expect: -8 */
/* expect: 7 */
/* expect: 4 */
/* expect: 13 */
/* expect: 31 */
/* expect: -2 */
/* expect: 6 */
/* expect: 1 */
/* expect: 72 */
/* expect: 9 */

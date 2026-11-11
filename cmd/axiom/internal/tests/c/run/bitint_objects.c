/* flags: --c-std=c23 */
/* Bit-precise integers as objects: members (layout), arrays, through
   pointers, static initializers, compound assignment with other types,
   conversions to and from floating types and bool, ++ and -- wrapping. */
#include "host.h"

struct S { char c; _BitInt(24) a; unsigned _BitInt(12) b; _BitInt(48) w; };
static unsigned _BitInt(10) tbl[3] = { 1023, 1024, -1 };
static _BitInt(20) sv = -524288;
static unsigned _BitInt(3) small = 9;

static void bump(unsigned _BitInt(12) *p) { *p += 4000; }

int main(void) {
    struct S s = { 'x', -1, 4095, 1 };
    print_i64(sizeof(struct S));
    print_i64(s.a);
    s.a += 8388607;
    print_i64(s.a);
    s.a++;
    print_i64(s.a);
    bump(&s.b);
    print_i64(s.b);
    s.w <<= 46;
    print_i64(s.w);
    s.w *= 2;
    print_i64(s.w);
    print_i64(tbl[0] + tbl[1] * 10000 + tbl[2]);
    print_i64(sv);
    print_i64(small);
    unsigned _BitInt(12) *p = &s.b;
    *p = 0;
    (*p)--;
    print_i64(s.b);
    double d = (_BitInt(30))-123456789;
    print_f64(d);
    print_i64((_BitInt(7))-63.9);
    int i = 1000;
    i += (_BitInt(4))-8;
    print_i64(i);
    unsigned _BitInt(6) u6 = 60;
    u6 += 1.5;
    print_i64(u6);
    print_i64((bool)(_BitInt(9))0 + (bool)(_BitInt(9))-256);
    print_i64((_BitInt(9))-256 < 0u);
    print_i64((unsigned _BitInt(9))511 == -1);
    return 0;
}

/* expect: 24 */
/* expect: -1 */
/* expect: 8388606 */
/* expect: 8388607 */
/* expect: 3999 */
/* expect: 70368744177664 */
/* expect: -140737488355328 */
/* expect: 2046 */
/* expect: -524288 */
/* expect: 1 */
/* expect: 4095 */
/* expect: -123456789 */
/* expect: -63 */
/* expect: 992 */
/* expect: 61 */
/* expect: 1 */
/* expect: 0 */
/* expect: 0 */

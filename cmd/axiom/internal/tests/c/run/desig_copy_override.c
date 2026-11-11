/* flags: --c-std=c99 */
/* 6.7.8p19: a designated initializer for a subobject overrides an earlier
   one for it. A structure copied in whole, then a brace list (or a string)
   for an aggregate inside it: that aggregate is initialized by the list,
   the rest of it zero; the rest of the copy is kept. A designator for a
   scalar inside the copy changes only that scalar. */
#include "host.h"

struct in { int a, b; };
struct mid { struct in i; int d; char s[4]; };
struct out { struct mid m; int c; };

int main(void) {
    struct mid mv = { { 1, 2 }, 3, "xyz" };
    struct out o = { .m = mv, .m.i = { 5 }, .c = 9 };
    struct out p = { mv, 7, .m.i.a = 8 };
    struct out q = { .m = mv, .m.s = "a" };
    struct out r = { .m = mv, .m.i = { .b = 6 }, .m.i.a = 4 };
    print_i64(o.m.i.a);
    print_i64(o.m.i.b);
    print_i64(o.m.d);
    print_i64(o.c);
    print_i64(p.m.i.a);
    print_i64(p.m.i.b);
    print_i64(p.m.d);
    print_i64(q.m.s[0] == 'a' && q.m.s[1] == 0 && q.m.s[2] == 0);
    print_i64(q.m.i.b);
    print_i64(r.m.i.a * 10 + r.m.i.b);
    print_i64(r.m.d);
    return 0;
}

/* expect: 5 */
/* expect: 0 */
/* expect: 3 */
/* expect: 9 */
/* expect: 8 */
/* expect: 2 */
/* expect: 3 */
/* expect: 1 */
/* expect: 2 */
/* expect: 46 */
/* expect: 3 */

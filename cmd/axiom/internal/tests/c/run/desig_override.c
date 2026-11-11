/* flags: --c-std=c99 */
/* Designated initializers that initialize a subobject again: a list,
   string or union member given later replaces what the subobject had
   (the rest of it is zero); a later scalar replaces only itself. */
#include "host.h"
union V { char c[4]; int i; };
struct S { int a; union V v; int z; };
static union V u3 = { .i = 0x01020304, .c[0] = 9 };
static union V u4 = { .c[1] = 5, .i = 7 };
static struct S s1 = { .v.i = -1, .v.c[2] = 3, .z = 1 };
static struct S s2 = { 1, { .c = "ab" }, 2, .v.c[1] = 'z' };
struct Q { int a[3]; int b; };
static struct Q q1 = { .a = { 1, 2, 3 }, .a[1] = 9, .b = 4 };
static struct Q q2 = { .a[2] = 5, .a = { 7 } };
static char cs[2][4] = { "ab", [0][3] = 'q', [1] = "xyz" };
int main(void) {
    union V l3 = { .i = 0x01020304, .c[0] = 9 };
    struct Q l2 = { .a[2] = 5, .a = { 7 } };
    print_i64(u3.i); print_i64(u4.i); print_i64(s1.v.i); print_i64(s1.z);
    print_i64(s2.v.c[0]); print_i64(s2.v.c[1]); print_i64(s2.v.c[2]); print_i64(s2.z);
    print_i64(q1.a[0] + q1.a[1] * 10 + q1.a[2] * 100 + q1.b * 1000);
    print_i64(q2.a[0] + q2.a[1] * 10 + q2.a[2] * 100);
    print_i64(l3.i);
    print_i64(l2.a[0] + l2.a[1] * 10 + l2.a[2] * 100);
    print_i64(cs[0][0] + cs[0][3] + cs[1][2]);
    return 0;
}
/* expect: 9 */
/* expect: 7 */
/* expect: 196608 */
/* expect: 1 */
/* expect: 97 */
/* expect: 122 */
/* expect: 0 */
/* expect: 2 */
/* expect: 4391 */
/* expect: 7 */
/* expect: 9 */
/* expect: 7 */
/* expect: 332 */

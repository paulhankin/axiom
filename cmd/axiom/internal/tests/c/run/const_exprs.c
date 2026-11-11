/* Constant expressions in the initializers of static objects (6.6p7-9):
   arithmetic constant expressions (with floating operands and casts
   between arithmetic types), null pointer constants, address constants
   (&, [], ., ->, *, casts of pointers, array and function designators) and
   address constants plus or minus integer constant expressions; an address
   constant converted to _Bool. */
#include "host.h"

struct S { int x, y; } s = { 1, 2 };
int a[4] = { 10, 20, 30, 40 };
static int f(void) { return 5; }

static double d = 1.0 / 4 + (float)0.5;
static int i = 1.5 * 2 + (int)2.9;
static unsigned long long big = sizeof(int) * 2.5;
static int *p1 = &a[2];
static int *p2 = a + 3;
static int *p3 = &a[1] - 1;
static int *p4 = &(&s)->y;
static int *p5 = &*&a[1];
static const char *str = "abcd" + 2;
static int (*fp1)(void) = f;
static int (*fp2)(void) = &f;
static int (*fp3)(void) = *f;
static int *np = 0 ? &a[0] : 0;
static int *cp = 1 ? &a[1] : 0;
static void *vp = (char *)a + 4;
static _Bool b1 = a;
static _Bool b2 = &s.y;
static _Bool b3 = (int *)0;
static _Bool b4 = 0.0;
static _Bool b5 = f;

int main(void) {
    print_f64(d);
    print_i64(i);
    print_u64(big);
    print_i64(*p1 + *p2 + *p3 + *p4 + *p5);
    print_i64(*str);
    print_i64(fp1() + fp2() + fp3());
    print_i64(np == 0);
    print_i64(*cp);
    print_i64(*(int *)vp);
    print_i64(b1 * 10000 + b2 * 1000 + b3 * 100 + b4 * 10 + b5);
    return 0;
}

/* expect: 0.75 */
/* expect: 5 */
/* expect: 10 */
/* expect: 102 */
/* expect: 99 */
/* expect: 15 */
/* expect: 1 */
/* expect: 20 */
/* expect: 20 */
/* expect: 11001 */

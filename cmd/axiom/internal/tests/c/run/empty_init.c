/* flags: --c-std=c23 */
/* C23 empty initializers: `{}` gives an object (a scalar too) the value
   it would have with static storage duration: zero, null; for a
   subobject inside a list, after a designator, and overriding an earlier
   value. */
#include "host.h"

struct B { unsigned a : 3; int b : 5; int *p; double d; };
static struct B sb = {};
static int si = {};
static int *sp = {};

int main(void) {
    int i = {};
    double d = {};
    int *p = {};
    struct B b = { 7, -3, &i, 1.5 };
    struct B z = {};
    struct B o = { .a = 5, .a = {}, .b = 9 };
    int arr[4] = { 1, {}, [3] = 4, [3] = {} };
    b = (struct B){};
    print_i64(i + (int)d + (p == 0) + si + (sp == 0));
    print_i64(sb.a + sb.b + (sb.p == 0) + (int)sb.d);
    print_i64(b.a + b.b + (b.p == 0));
    print_i64(z.a + z.b + (int)z.d);
    print_i64(o.a * 100 + o.b);
    print_i64(arr[0] + arr[1] + arr[2] + arr[3]);
    return 0;
}
/* expect: 2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 9 */
/* expect: 1 */

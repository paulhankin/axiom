/* flags: --c-std=gnu11 */
/* expect-warning: C4143 */
/* A GNU dialect takes a cast of a structure or union value to its own type
   (6.5.4p2 asks for a scalar type), with a warning: the value, not an lvalue,
   of the type without qualifiers. */
#include "host.h"

struct s { int a; char t[3]; };
union u { int i; float f; };

int main(void) {
    struct s x = { 4, "ab" };
    const struct s cx = { 5, "cd" };
    struct s y = (struct s)x;
    struct s z = (struct s)cx;            /* a qualified operand */
    struct s w[2] = { (struct s)x, (const struct s)z };
    union u v = { 9 };
    union u v2 = (union u)v;
    y.a += 1;
    print_i64(x.a + y.a + z.a + w[0].a + w[1].a + v2.i);
    print_i64(w[1].t[0]);
    print_i64(sizeof((struct s)x));
    return 0;
}
/* expect: 32 */
/* expect: 99 */
/* expect: 8 */

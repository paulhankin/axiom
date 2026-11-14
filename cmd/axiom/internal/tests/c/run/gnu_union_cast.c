/* flags: --c-std=gnu17 */
/* A GNU dialect's cast to a union type: a value of the type of one of its
   members (a structure too) cast to the union is the union's value with
   that member given it (its first member of that type), not an lvalue; an
   argument, an initializer, an operand of '.'. */
#include "host.h"

struct pair { int a, b; };
union u { int i; double d; struct pair p; long l; };

static long pick(union u x, int k) {
    return k == 0 ? x.i : k == 1 ? (long)(x.d * 4) : x.p.a * 10 + x.p.b;
}

int main(void) {
    double d = 2.5;
    struct pair q = { 3, 4 };
    const int c = 9;
    union u a = (union u)d;
    print_i64(pick((union u)7, 0));
    print_i64(pick(a, 1));
    print_i64(pick((union u)q, 2));
    print_i64(((union u)c).i);
    print_i64(((union u)5L).l);
    return 0;
}
/* expect: 7 */
/* expect: 10 */
/* expect: 34 */
/* expect: 9 */
/* expect: 5 */

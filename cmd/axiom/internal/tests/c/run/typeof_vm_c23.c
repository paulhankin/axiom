/* flags: --c-std=c23 */
/* typeof of a variably modified type (C23 6.7.2.5): its operand is
   evaluated, a type name's array lengths where the declaration (or the
   sizeof, the cast) is reached, an expression as such; typeof of any other
   type does not evaluate its operand. typeof_unqual of an array of const
   elements is an array of unqualified ones (an array and its elements are
   qualified alike). */
#include "host.h"

static int calls;
static int f(void) { calls++; return 4; }

int main(void) {
    int n = 3;
    typeof(int[n++]) a;
    print_i64(n);
    print_i64(sizeof a);
    typeof(char[f()]) b;
    print_i64(calls);
    print_i64(sizeof b);
    int (*p)[n] = 0;
    typeof(p[calls++]) c;
    print_i64(calls);
    print_i64(sizeof c);
    typeof(n++) d = 0;
    print_i64(n);
    int vla[n];
    typeof(vla) e;
    print_i64(sizeof e);
    print_i64(sizeof(typeof(char[f() + 1])));
    print_i64(calls);
    const int cv[n + 1];
    typeof_unqual(cv) u;
    u[0] = 7;
    print_i64(sizeof u + u[0]);
    typedef typeof(short[n]) S;
    n = 100;
    S s;
    print_i64(sizeof s);
    print_i64(sizeof(*(typeof(int (*)[f()]))p));
    print_i64(calls);
    (void)d;
    return 0;
}
/* expect: 4 */
/* expect: 12 */
/* expect: 1 */
/* expect: 4 */
/* expect: 2 */
/* expect: 16 */
/* expect: 4 */
/* expect: 16 */
/* expect: 5 */
/* expect: 3 */
/* expect: 27 */
/* expect: 8 */
/* expect: 16 */
/* expect: 4 */

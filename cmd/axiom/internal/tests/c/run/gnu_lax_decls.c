/* flags: --c-std=gnu17 */
/* expect-warning: C4146 */
/* expect-warning: C4147 */
/* A GNU dialect accepts what the common compilers' default modes accept:
   an empty declaration at file scope (no diagnostic), pointers to
   incompatible object types converted in an assignment, initialization,
   argument or return (C4146, the conversion a cast makes), and main
   defined as int main(int argc) (C4147), which receives argc. */
#include "host.h"
;
struct a { int x; };
struct b { int y; };
static int first(struct a *p) { return p->x; }
static long *as_long(int *p) { return p; } /* expect-warning: C4146 */
;;
int main(int argc) { /* expect-warning: C4147 */
    struct b v = { 41 };
    int n = 7;
    long *q = &n; /* expect-warning: C4146 */
    double m[2][3] = { { 1, 2, 3 }, { 4, 5, 6 } };
    double (*r)[2] = m; /* expect-warning: C4146 */
    print_i64(first(&v) + 1); /* expect-warning: C4146 */
    print_i64(as_long(&n) == q);
    print_i64((long)(*r)[1]);
    print_i64(argc >= 0);
    return 0;
}
/* expect: 42 */
/* expect: 1 */
/* expect: 2 */
/* expect: 1 */

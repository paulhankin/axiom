/* flags: --c-std=c11 */
/* Declarations: repeated qualifiers (allowed from C99, directly and through
   typedefs), register objects and parameters, _Noreturn and inline
   functions, static assertions with a message at file scope, in blocks and
   in structures, a union initialized through its first named member
   (unnamed bit-fields take no part), braces around scalars. */
#include "host.h"

typedef const int cint;
const cint twice_const = 4;
const const volatile int many = 5;

union U { int : 4; short s; char c[2]; };
struct S { int : 3; int a; _Static_assert(sizeof(int) >= 2, "int is too small"); };

_Static_assert(sizeof(struct S) >= sizeof(int), "struct S");
static union U u = { 0x0102 };
static struct S st = { 9 };

static inline int sq(register int x) { return x * x; }
_Noreturn static void stop(void) { for (;;) { } }

int main(void) {
    register int r = 3;
    register struct { int v; } rs = { 11 };
    int braced = { 6 };
    _Static_assert(1, "in a block");
    if (r == 0) {
        stop();
    }
    print_i64(twice_const + many);
    print_i64(u.s);
    print_i64(st.a);
    print_i64(sq(r) + rs.v + braced);
    return 0;
}
/* expect: 9 */
/* expect: 258 */
/* expect: 9 */
/* expect: 26 */

/* flags: --c-std=c89 --c-int=32 */
/* What 6.6p4 allows: unsigned arithmetic wraps; an operand not evaluated
   may overflow; conversions to narrower types are not operations that
   overflow; and C89 shifts a signed value's bits. */
#include "host.h"

unsigned a = 4294967295u + 2;
int b = 0 && (2147483647 + 1);
int c = 1 ? 5 : 2147483647 * 2;
short d = (short)70000;
int e = 1 << 31;

int main(void) {
    print_i64(a);
    print_i64(b);
    print_i64(c);
    print_i64(d);
    print_i64(e);
    return 0;
}

/* expect: 1 */
/* expect: 0 */
/* expect: 5 */
/* expect: 4464 */
/* expect: -2147483648 */

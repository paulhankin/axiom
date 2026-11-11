/* flags: --c-int=64 --c-long=64 */
/* Bit-fields with a 64-bit int: units of 8 bytes, widths up to 64. */
#include "host.h"

struct A { int a : 40; int b : 30; unsigned c : 64; };
struct B { char c; int x : 8; };

int main(void) {
    struct A s = { -1, 536870911, 18446744073709551615u };
    print_i64(sizeof(struct A));
    print_i64(_Alignof(struct A));
    print_i64(sizeof(struct B));
    print_i64(s.a);
    print_i64(s.b);
    s.b += 1;
    print_i64(s.b);
    print_u64(s.c);
    s.a = 549755813887;
    print_i64(s.a);
    s.a++;
    print_i64(s.a);
    return 0;
}
/* expect: 24 */
/* expect: 8 */
/* expect: 8 */
/* expect: -1 */
/* expect: 536870911 */
/* expect: -536870912 */
/* expect: 18446744073709551615 */
/* expect: 549755813887 */
/* expect: -549755813888 */

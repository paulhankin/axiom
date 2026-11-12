/* flags: --c-std=c23 */
/* Bit-fields of bit-precise types (C23 _BitInt): layout in units of their
   storage, sign extension, increment and compound assignment, static and
   automatic initializers. */
#include "host.h"
struct S { unsigned _BitInt(10) a : 7; _BitInt(20) b : 12; _BitInt(50) c : 40; int d : 3; };
static struct S g = { 100, -2000, -5, 3 };
int main(void) {
    struct S s = { 127, 2047, 1, -1 };
    s.a++; s.b++;
    print_i64(s.a); print_i64(s.b); print_i64((long long)s.c); print_i64(s.d);
    print_i64(g.a); print_i64(g.b); print_i64((long long)g.c); print_i64(g.d);
    return 0;
}
/* expect: 0 */
/* expect: -2048 */
/* expect: 1 */
/* expect: -1 */
/* expect: 100 */
/* expect: -2000 */
/* expect: -5 */
/* expect: 3 */

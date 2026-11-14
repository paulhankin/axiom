/* flags: --c-std=gnu17 */
/* Bit-fields of types wider than int (6.7.2.1p5 lets an implementation
   allow them) promote by their width (6.3.1.1p2): to int when int holds
   every value of the width, else to unsigned int when that does, else
   they keep their type; so x - 2 of a zero one is negative for the first,
   not for the others, and sizeof the promoted value says which. */
#include "host.h"

struct w {
    unsigned long u31 : 31;
    unsigned long u32 : 32;
    long s32 : 32;
    unsigned long long ull3 : 3;
    unsigned long long ull35 : 35;
    long long s40 : 40;
} x;

int main(void) {
    print_i64((x.u31 - 2) < 0);
    print_i64((x.u32 - 2) < 0);
    print_i64((x.s32 - 2) < 0);
    print_i64((x.ull3 - 2) < 0);
    print_i64((x.ull35 - 2) < 0);
    print_i64((x.s40 - 2) < 0);
    print_i64(sizeof(+x.u31) * 100 + sizeof(+x.u32) * 10 + sizeof(+x.ull35));
    x.s32 = -5;
    print_i64(x.s32 * 3);
    return 0;
}
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 448 */
/* expect: -15 */

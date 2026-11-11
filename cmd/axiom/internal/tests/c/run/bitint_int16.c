/* flags: --c-std=c23 --c-int=16 --c-long=32 */
/* The ranks of bit-precise integer types against the standard ones of the
   same and other widths (6.3.1.1p1) with --c-int=16 --c-long=32. */
#include "host.h"

#define T(x) _Generic((x), int: 1, unsigned: 2, long: 3, unsigned long: 4, _BitInt(16): 5, \
    unsigned _BitInt(16): 6, _BitInt(40): 7, unsigned _BitInt(40): 8, _BitInt(64): 9, default: 0)

int main(void) {
    print_i64(T((_BitInt(16))1 + 1));
    print_i64(T((unsigned _BitInt(16))1 + 1));
    print_i64(T((_BitInt(40))1 + 1));
    print_i64(T((unsigned _BitInt(40))1 + 1));
    print_i64(T((_BitInt(40))1 + 1u));
    print_i64(T((_BitInt(64))1 + 1L));
    print_i64(T((unsigned _BitInt(16))1 + (_BitInt(16))1));
    print_i64((unsigned _BitInt(16))0 - 1 > 0);
    return 0;
}

/* expect: 1 */
/* expect: 2 */
/* expect: 7 */
/* expect: 8 */
/* expect: 7 */
/* expect: 9 */
/* expect: 6 */
/* expect: 1 */

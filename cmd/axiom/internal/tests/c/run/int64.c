/* flags: --c-int=64 --c-long=64 */
/* A 64-bit int (and so long): limits, promotions of the narrower types,
   constants, and 32-bit unsigned arithmetic that no longer wraps in int. */
#include "host.h"
#include <limits.h>

static int sq(int x) { return x * x; }

int main(void) {
    unsigned short us = 65535;
    unsigned char uc = 255;
    int i = 4000000000;
    print_i64(sizeof(int));
    print_i64(sizeof(long));
    print_i64(INT_MAX);
    print_i64(INT_MIN);
    print_u64(UINT_MAX);
    print_i64(sq(100000));
    print_i64(us * us);
    print_i64(uc << 40);
    print_i64(i);
    print_i64(sizeof 2147483648);
    print_i64(sizeof 'x');
    print_i64(sizeof(1 == 1));
    print_i64(-1 < 0u);
    print_u64(0u - 1);
    print_i64(sizeof(size_t));
    return 0;
}

/* expect: 8 */
/* expect: 8 */
/* expect: 9223372036854775807 */
/* expect: -9223372036854775808 */
/* expect: 18446744073709551615 */
/* expect: 10000000000 */
/* expect: 4294836225 */
/* expect: 280375465082880 */
/* expect: 4000000000 */
/* expect: 8 */
/* expect: 8 */
/* expect: 8 */
/* expect: 0 */
/* expect: 18446744073709551615 */
/* expect: 8 */

/* flags: --c-long=64 */
/* A 64-bit long on every target (on wasm32 too, where it is wider than a
   pointer): its limits, constants, and conversions with pointers. */
#include "host.h"
#include <limits.h>

int main(void) {
    long l = LONG_MAX;
    unsigned long ul = ULONG_MAX;
    int a[4] = { 1, 2, 3, 4 };
    long k = 2;
    print_i64(sizeof(long));
    print_i64(l);
    print_u64(ul);
    print_i64(LONG_MIN);
    print_i64(sizeof 2147483648);
    print_i64(sizeof 4294967296);
    print_i64(a[k]);
    print_i64(*(a + k + 1));
    print_i64(-1L < 0u);
    print_i64(l + 1 < 0);
    return 0;
}

/* expect: 8 */
/* expect: 9223372036854775807 */
/* expect: 18446744073709551615 */
/* expect: -9223372036854775808 */
/* expect: 8 */
/* expect: 8 */
/* expect: 3 */
/* expect: 4 */
/* expect: 1 */
/* expect: 1 */

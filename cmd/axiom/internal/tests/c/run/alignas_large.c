/* flags: --c-std=c11 */
/* The strictest alignment supported (4096) for objects with static and
   automatic storage duration. */
#include "host.h"

_Alignas(4096) static char page[10];
_Alignas(4096) int pg2 = 3;

static unsigned long low_bits(const void *p) { return (unsigned long)p & 4095; }

int main(void) {
    _Alignas(4096) char loc[3];
    loc[0] = 1;
    page[0] = 2;
    print_i64(low_bits(page) + low_bits(&pg2) + low_bits(loc));
    print_i64(loc[0] + page[0] + pg2);
    return 0;
}
/* expect: 0 */
/* expect: 6 */

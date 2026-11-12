/* In C23 mode (C23 7.24.1.7), strtol and its kin take the 0b and 0B
   prefixes for base 0 and 2 (and still none for other bases). */
/* flags: --c-std=c23 */
#include <stdlib.h>
#include "print.h"

int main(void) {
    char *end;
    const char *s = "0b1011z";
    print_i64(strtol(s, &end, 0)); /* expect: 11 */
    print_i64(end - s); /* expect: 6 */
    print_i64(strtoll("-0B11", nullptr, 2)); /* expect: -3 */
    s = "0b";
    print_u64(strtoull(s, &end, 0)); /* expect: 0 */
    print_i64(end - s); /* expect: 1 */
    print_u64(strtoul("0b12", nullptr, 0)); /* expect: 1 */
    print_i64(strtol("0b11", &end, 16)); /* expect: 2833 */
    print_i64(strtol("0b11", &end, 10)); /* expect: 0 */
    return 0;
}

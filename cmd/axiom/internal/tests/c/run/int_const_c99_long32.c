/* flags: --c-std=c99 --c-int=32 --c-long=32 */
/* C99 6.4.4.1p5: an unsuffixed decimal constant is int, long or long long;
   an octal or hexadecimal one int, unsigned int, long, unsigned long, long
   long or unsigned long long: a decimal constant too large for long is
   long long here, not unsigned long. */
#include "host.h"

int main(void) {
    print_i64(sizeof(2147483647));
    print_i64(sizeof(2147483648));
    print_i64(2147483648 > 0);
    print_i64(-2147483648 > 0);
    print_i64(3000000000 > 0);
    print_i64(sizeof(4294967295));
    print_i64(0x7fffffff > 0 && sizeof(0x7fffffff) == sizeof(int));
    print_i64(-0x80000000 > 0);
    print_i64(017777777777 == 2147483647);
    print_i64(-020000000000 > 0);
    print_i64(-2147483648L > 0);
    print_i64(-1 < 2147483648);
    print_i64(-1L < 4294967295);
    print_i64(sizeof(1u) == sizeof(int) && sizeof(4294967295u) == sizeof(int));
    return 0;
}

/* expect: 4 */
/* expect: 8 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 8 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */

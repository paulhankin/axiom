/* A fallthrough declaration that ends a block: the next block item reached
   after it is the case or default label after the block (C23 6.7.13.5p2),
   through nested blocks and the arms of an if statement. Both spellings,
   the standard attribute and the GNU one. */
/* flags: --c-std=gnu23 */
#include "host.h"
static int f(int x, int y) {
    int r = 0;
    switch (x) {
    case 0: {
        r += 1;
        [[fallthrough]];
    }
    case 1: {
        r += 10;
        {
            if (y) {
                r += 1000;
                __attribute__((fallthrough));
            } else {
                r += 2000;
                [[fallthrough]];
            }
        }
    }
    default:
        r += 100;
        break;
    case 5:
        if (y) {
            r = -1;
            [[fallthrough]];
        }
    case 6:
        r -= 5;
    }
    return r;
}
int main(void) {
    print_i64(f(0, 0));
    print_i64(f(0, 1));
    print_i64(f(1, 1));
    print_i64(f(2, 0));
    print_i64(f(5, 1));
    print_i64(f(5, 0));
    print_i64(f(6, 0));
    return 0;
}

/* expect: 2111 */
/* expect: 1111 */
/* expect: 1110 */
/* expect: 100 */
/* expect: -6 */
/* expect: -5 */
/* expect: -5 */

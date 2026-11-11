/* C23: fallthrough declarations before case and default labels. */
/* flags: --c-std=c23 */
#include "host.h"
static int f(int x) {
    int r = 0;
    switch (x) {
    case 1:
        r += 1;
        [[fallthrough]];
    case 2:
        r += 10;
        [[__fallthrough__]];
    default:
        r += 100;
        break;
    case 3: [[fallthrough]];
    case 4:
        r = -1;
    }
    return r;
}
int main(void) {
    for (int i = 0; i < 5; i++) print_i64(f(i));
    return 0;
}

/* expect: 100 */
/* expect: 111 */
/* expect: 110 */
/* expect: -1 */
/* expect: -1 */

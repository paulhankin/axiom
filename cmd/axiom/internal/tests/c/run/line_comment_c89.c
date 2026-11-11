/* flags: --c-std=c89 */
/* In C89, //* starts a division and a comment. */
#include "host.h"

int main(void) {
    int a = 6 //* a comment */ 2
    ;
    print_i64(a);
    return 0;
}

/* expect: 3 */

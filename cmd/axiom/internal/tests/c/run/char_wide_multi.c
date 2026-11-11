/* flags: --c-std=c89 */
/* expect-warning: C4102 */
/* 6.4.4.4p11: a wide character constant of more than one character has an
   implementation-defined value: here its last character's. */
#include "host.h"

int main(void) {
    print_i64(L'ab');
    print_i64(L'xyz' == L'z');
    return 0;
}

/* expect: 98 */
/* expect: 1 */

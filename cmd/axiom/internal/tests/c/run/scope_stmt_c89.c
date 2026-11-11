/* flags: --c-std=c89 */
/* C89: an if or while statement is not a block of its own; what its
   controlling expression declares is in the enclosing block's scope. */
#include "host.h"

int main(void) {
    int n = 0;
    if (sizeof(enum { a = 2 }) > 0) n = a;
    while (n < 5 && sizeof(enum { b = 3 })) n += b;
    print_i64(n + a + b);
    return 0;
}

/* expect: 10 */

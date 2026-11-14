/* flags: --c-std=gnu17 */
/* A GNU dialect takes these C23 forms in every standard, as the common
   compilers' default modes do, without a diagnostic: a function definition
   with unnamed parameters, labels before declarations and at the end of a
   block, and a static assertion without a message. */
#include "host.h"
_Static_assert(sizeof(int) >= 2);
static int second(int, int b) { return b; }
static int count(int n) {
    int r = 0;
again:
    int k = n;
    r += k;
    if (--n > 0) {
        goto again;
    }
    {
        goto out;
    out:
    }
    return r;
}
int main(void) {
    print_i64(second(1, 42));
    print_i64(count(4));
    return 0;
}
/* expect: 42 */
/* expect: 10 */

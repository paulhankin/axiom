/* flags: --c-std=gnu99 */
/* linux-only */
/* The address of a weak function declared without a prototype, which no
   unit defines and nothing calls before: a null pointer, in an automatic
   object's initializer (the first use of the function), a static one's and
   a comparison (the function's
   declaration has no parameter types to give it a signature of its own;
   6.7.6.3p14). It is called only when not null. */
#include "host.h"

extern int absent_noproto() __attribute__((weak));
extern int absent_noproto2() __attribute__((weak));
static int (*sp)() = absent_noproto2;

int main(void) {
    int (*t)() = absent_noproto;
    print_i64(t == 0);
    print_i64(sp == 0);
    print_i64(absent_noproto == 0);
    if (t)
        return t(1, 2);
    return 0;
}
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */

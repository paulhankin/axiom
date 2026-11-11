/* Falling off the end of main returns 0 (C99 5.1.2.2.3); falling off the end
   of another function is fine when the caller does not use the value. */
#include "host.h"
static int no_value(int x) {
    if (x > 0) return x;
    print_i64(-1);
}  /* falls off the end: fine while the caller does not use the value */
int main(void) {
    no_value(0);
    print_i64(no_value(5));
}

/* expect: -1 */
/* expect: 5 */

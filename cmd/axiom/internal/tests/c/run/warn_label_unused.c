/* A label that no goto names is reported (a warning: the program runs). */
/* expect-warning: C4040 */
#include "host.h"
int main(void) {
    int x = 1;
unused:
    x++;
    print_i64(x);
    return 0;
}

/* expect: 2 */

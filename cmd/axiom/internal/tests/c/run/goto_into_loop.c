/* goto into the body of a loop: control flow with a second entry into the loop. */
#include "host.h"
int main(void) {
    int i = 0, n = 0;
    goto inside;
    while (i < 5) {
        n += 10;
    inside:
        n++;
        i++;
    }
    print_i64(n);
    return 0;
}

/* expect: 45 */

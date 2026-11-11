/* flags: --c-std=c11 */
/* C11 6.5.8p2: pointers to compatible object types, complete or not. */
#include "host.h"

int a[3];
int (*p)[] = &a;
int (*q)[3] = &a;
int main(void) {
    print_i64(p <= q);
    return 0;
}

/* expect: 1 */

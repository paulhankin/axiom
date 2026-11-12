/* flags: --c-std=c23 */
/* C23 declares call_once, once_flag and ONCE_FLAG_INIT in <stdlib.h> too
   (7.24.2.1): no <threads.h> here. */
#include <stdio.h>
#include <stdlib.h>

static once_flag f = ONCE_FLAG_INIT;
static int n;

static void count(void) {
    n++;
}

int main(void) {
    call_once(&f, count);
    call_once(&f, count);
    printf("%d\n", n); /* expect: 1 */
    return 0;
}

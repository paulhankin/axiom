/* flags: --c-std=gnu17 */
/* In a GNU dialect <string.h> declares strdup and strndup in every
   standard, <stdlib.h> posix_memalign (an object aligned to a power of two
   that is a multiple of sizeof(void *), else EINVAL, which free takes) and
   alloca, and <alloca.h> alloca: the frame's memory, until the function
   returns. */
#include "print.h"
#include <alloca.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int frame(int n) {
    char *p = alloca(n);
    memset(p, 7, n);
    return p[n - 1] + (int)((uintptr_t)p % 16 == 0);
}

int main(void) {
    char *d = strdup("abc");
    char *e = strndup("abcdef", 2);
    print_i64(strlen(d) * 10 + strlen(e));
    free(d);
    free(e);
    void *p = 0;
    print_i64(posix_memalign(&p, 64, 100));
    print_i64((uintptr_t)p % 64);
    free(p);
    p = 0;
    print_i64(posix_memalign(&p, 3 * sizeof(void *), 8) == EINVAL && p == 0);
    print_i64(posix_memalign(&p, sizeof(void *) / 2, 8) == EINVAL);
    print_i64(frame(10) + frame(1000));
    return 0;
}
/* expect: 32 */
/* expect: 0 */
/* expect: 0 */
/* expect: 1 */
/* expect: 1 */
/* expect: 16 */

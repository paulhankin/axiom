/* C23's free_sized, free_aligned_sized (7.24.3.4, 7.24.3.5) and
   memalignment (7.24.9.1). */
/* flags: --c-std=c23 */
#include <stdlib.h>
#include "print.h"

int main(void) {
    char *p = malloc(40), *q = aligned_alloc(256, 512);
    static long long arr[4];
    print_bool(memalignment(p) >= 16); /* expect: true */
    print_bool(memalignment(q) >= 256); /* expect: true */
    print_i64((long long)memalignment(nullptr)); /* expect: 0 */
    print_i64((long long)memalignment((char *)q + 8) == 8); /* expect: 1 */
    print_bool(memalignment(&arr[1]) >= 8); /* expect: true */
    free_sized(p, 40);
    free_aligned_sized(q, 256, 512);
    free_sized(nullptr, 0);
    print_cstr("done"); /* expect: done */
    return 0;
}

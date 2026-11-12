/* C23 7.24.5.1: bsearch's result points to const when the array's pointer
   does (a type-generic macro), and the function itself is still there. */
/* flags: --c-std=c23 */
#include <stdlib.h>
#include "print.h"

static int cmp(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

int main(void) {
    static const int ca[4] = {1, 3, 5, 7};
    static int ma[4] = {2, 4, 6, 8};
    int k = 5, j = 6;
    auto p = bsearch(&k, ca, 4, sizeof ca[0], cmp);
    auto q = bsearch(&j, ma, 4, sizeof ma[0], cmp);
    void *(*f)(const void *, const void *, size_t, size_t, int (*)(const void *, const void *)) = bsearch;
    print_i64(_Generic(p, const void *: 1, void *: 2, default: 3)); /* expect: 1 */
    print_i64(_Generic(q, const void *: 1, void *: 2, default: 3)); /* expect: 2 */
    print_i64(*(const int *)p); /* expect: 5 */
    print_i64(*(int *)f(&j, ma, 4, sizeof ma[0], cmp)); /* expect: 6 */
    return 0;
}

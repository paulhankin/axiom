/* flags: --c-std=gnu17 */
/* In a hosted GNU dialect a call of alloca that the program does not
   define is __builtin_alloca (n bytes of the caller's frame), declared or
   not, as the C library's <alloca.h> would make it; each call is another
   block. */
#include <stddef.h>
#include "print.h"

void *alloca(size_t);

static int sum(int n) {
    int *a = alloca(n * sizeof(int));
    int *b = (int *)alloca(sizeof(int));
    int s = 0;
    for (int i = 0; i < n; i++)
        a[i] = i;
    *b = 100;
    for (int i = 0; i < n; i++)
        s += a[i];
    return s + *b;
}

int main(void) {
    print_i64(sum(10)); /* expect: 145 */
    return 0;
}

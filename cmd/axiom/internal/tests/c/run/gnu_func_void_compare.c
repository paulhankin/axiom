/* flags: --c-std=gnu17 */
/* expect-warning: C4149 */
/* A GNU dialect compares a pointer to void with a pointer to a function
   (6.5.9p2 allows neither order), with the warning C4149: the function
   pointer is converted to the void pointer's type, as a cast converts it. */
#include "host.h"
static int one(void) { return 1; }
static int two(void) { return 2; }
int main(void) {
    void *p = (void *)one;
    int (*q)(void) = two;
    print_bool(p == one); /* expect-warning: C4149 */
    print_bool(two != p); /* expect-warning: C4149 */
    print_bool(p == q); /* expect-warning: C4149 */
    return 0;
}
/* expect: true */
/* expect: true */
/* expect: false */

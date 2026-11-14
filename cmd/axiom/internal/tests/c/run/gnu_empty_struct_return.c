/* flags: --c-std=gnu17 */
/* A function returning a structure without members (a GNU dialect's, of
   size 0), directly and through a pointer, and one taking such arguments
   between others: on AArch64 its result is in the memory x8 points to,
   of which no byte is stored. */
#include "host.h"

typedef struct {} E;
struct W { E e; int x; };

static E make(void) {
    E e;
    return e;
}
static int take(E a, int b, E c, int d) {
    (void)a;
    (void)c;
    return b * 10 + d;
}
E (*fp)(void) = make;

int main(void) {
    E e = make();
    E f = fp();
    struct W w = { f, 3 };
    print_i64(take(e, 4, f, 5));
    print_i64(w.x + sizeof(E));
    return 0;
}
/* expect: 45 */
/* expect: 3 */

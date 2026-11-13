/* flags: --c-std=gnu17 */
/* expect-trap */
/* `goto *` to a label in the scope of a variable length array that the
   goto is not in: a jump into that scope (6.8.6.1p1), which traps. */
#include "host.h"

static int into(int n, void **out) {
    void *t = &&in;
    *out = t;
    if (n > 0)
        goto *t;
    {
        int a[n + 4];
        a[0] = 1;
    in:
        return a[0];
    }
}

int main(void) {
    void *p;
    return into(4, &p);
}

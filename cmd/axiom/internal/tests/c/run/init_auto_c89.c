/* flags: --c-std=c89 */
/* C89: an automatic aggregate is initialized by a list of constants, or a
   structure by a single expression of its type; a scalar by any expression,
   in braces or not. Its identifier list is a definition's. */
#include "host.h"

struct s { int a, b; };

static struct s make(x) int x; {
    struct s v;
    v.a = x;
    v.b = x * 2;
    return v;
}

int main(void) {
    int y = 3;
    int z = { y + 1 };
    struct s v = make(y);
    struct s w = { 1, sizeof(struct s) };
    int a[3] = { 1, 'b', (int)2.5 };
    print_i64(z);
    print_i64(v.b);
    print_i64(w.b == sizeof(struct s));
    print_i64(a[0] + a[1] + a[2]);
    return 0;
}

/* expect: 4 */
/* expect: 6 */
/* expect: 1 */
/* expect: 101 */

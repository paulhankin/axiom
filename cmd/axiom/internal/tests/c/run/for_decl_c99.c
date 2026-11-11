/* flags: --c-std=c99 */
/* 6.8.5p3: a for statement's declaration may use a tag declared before, or
   an unnamed structure; its objects are auto or register. */
#include "host.h"

struct t { int v; };

int main(void) {
    struct t a = { 4 };
    int n = 0;
    for (struct t *p = &a; p; p = 0) n += p->v;
    for (struct { int a, b; } x = { 1, 2 }; x.a < 3; x.a++) n += x.b;
    for (register int i = 0, j = 10; i < j; i += 5) n++;
    print_i64(n);
    return 0;
}

/* expect: 10 */

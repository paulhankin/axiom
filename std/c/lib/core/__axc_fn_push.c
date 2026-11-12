/* __axc_fn_push: adds f to a table of functions to call at exit (atexit,
   at_quick_exit): 32 in the table itself (the least the standard asks for,
   C23 7.24.4.3p4), then blocks of 32 more from malloc. 0, or nonzero when
   there is no memory for another block. */
#include <stdlib.h>
#include "axc/core_impl.h"

int __axc_fn_push(struct __axc_fn_table *t, void (*f)(void)) {
    struct __axc_fn_block *b = t->top ? t->top : &t->first;
    if (t->n == 32) {
        struct __axc_fn_block *nb = malloc(sizeof *nb);
        if (!nb)
            return 1;
        nb->prev = b;
        t->top = nb;
        t->n = 0;
        b = nb;
    }
    b->fns[t->n++] = f;
    return 0;
}

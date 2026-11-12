/* __axc_fn_push: adds f to a table of functions to call at exit (atexit,
   at_quick_exit): 32 in the table itself (the least the standard asks for,
   C23 7.24.4.3p4), then blocks of 32 more from malloc. 0, or nonzero when
   there is no memory for another block. Thread-safe (the table's lock). */
#include <stdlib.h>
#include "axc/core_impl.h"

int __axc_fn_push(struct __axc_fn_table *t, void (*f)(void)) {
    struct __axc_fn_block *b, *nb = (struct __axc_fn_block *)0;
    for (;;) {
        __axc_fn_lock(t);
        b = t->top ? t->top : &t->first;
        if (t->n < 32 || nb)
            break;
        /* (the block allocated without the lock, which malloc may need) */
        t->lock = 0;
        nb = malloc(sizeof *nb);
        if (!nb)
            return 1;
    }
    if (t->n == 32) {
        nb->prev = b;
        t->top = nb;
        t->n = 0;
        b = nb;
    } else if (nb) {
        free(nb); /* (another thread made room meanwhile) */
    }
    b->fns[t->n++] = f;
    t->lock = 0;
    return 0;
}

/* __axc_fn_run: calls the functions of a table, the last registered first
   (C23 7.24.4.4p3, 7.24.4.7p3). Each is taken out before it is called, so
   that a function that calls exit (or registers more: those run next) does
   not make the process hang or call one twice. The blocks past the first
   are not given back: the process is ending (and exit then needs no
   allocator). */
#include "axc/core_impl.h"

void __axc_fn_run(struct __axc_fn_table *t) {
    for (;;) {
        struct __axc_fn_block *b;
        void (*f)(void);
        __axc_fn_lock(t);
        b = t->top ? t->top : &t->first;
        if (t->n == 0) {
            if (!t->top) {
                t->lock = 0;
                return;
            }
            /* (the block before an emptied one is full) */
            t->top = b->prev == &t->first ? (struct __axc_fn_block *)0 : b->prev;
            t->n = 32;
            t->lock = 0;
            continue;
        }
        f = b->fns[--t->n];
        /* (called without the lock: it may register more, or exit) */
        t->lock = 0;
        f();
    }
}

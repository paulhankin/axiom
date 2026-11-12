/* __axc_fn_lock: takes the lock of a table of functions to call at exit
   (atexit, at_quick_exit), which threads may change at once: a spin on an
   atomic exchange, as it is held only while one entry is added or taken
   out (never while a function runs). Uncontended, one exchange. */
#include <stdatomic.h>
#include "axc/core_impl.h"

void __axc_fn_lock(struct __axc_fn_table *t) {
    while (atomic_exchange(&t->lock, 1))
        ;
}

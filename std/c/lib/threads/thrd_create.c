/* thrd_create (C17 7.26.5.1): a new thread of the Axiom runtime runs
   func(arg); *thr identifies it. thrd_nomem if there is no memory for it,
   thrd_error if it cannot be started (also on WebAssembly without the
   atomics extension: std/c/rt/threads_spawn_none.ax). */
#include "axc/threads_impl.h"

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg) {
    return (int)__axc_thrd_spawn(func, arg, thr);
}

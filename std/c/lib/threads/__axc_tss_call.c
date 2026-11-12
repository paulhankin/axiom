/* __axc_tss_call: calls a thread-specific storage destructor with its
   value, for std/c/rt/threads.ax (at a thread's exit, 7.26.6.1). */
#include "axc/threads_impl.h"

void __axc_tss_call(tss_dtor_t dtor, void *p) {
    dtor(p);
}

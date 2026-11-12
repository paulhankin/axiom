/* tss_get (C17 7.26.6.4): the calling thread's value for key (null if it
   set none, or key was deleted). */
#include "axc/threads_impl.h"

void *tss_get(tss_t key) {
    return __axc_tss_get(key);
}

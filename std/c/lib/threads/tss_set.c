/* tss_set (C17 7.26.6.5): sets the calling thread's value for key;
   thrd_error if there is no memory for it. */
#include "axc/threads_impl.h"

int tss_set(tss_t key, void *val) {
    return (int)__axc_tss_set(key, val);
}

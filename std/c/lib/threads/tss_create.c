/* tss_create (C17 7.26.6.2): a new key, with the destructor dtor (or none
   if it is null), whose value is null in every thread. thrd_error if there
   is no memory for it. */
#include "axc/threads_impl.h"

int tss_create(tss_t *key, tss_dtor_t dtor) {
    return (int)__axc_tss_create(key, dtor, dtor != 0);
}

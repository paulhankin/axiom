/* tss_delete (C17 7.26.6.3): gives key back; no destructor is called. */
#include "axc/threads_impl.h"

void tss_delete(tss_t key) {
    __axc_tss_delete(key);
}

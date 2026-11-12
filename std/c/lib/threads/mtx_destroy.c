/* mtx_destroy (C17 7.26.4.1): mtx holds no resources to release. */
#include "axc/threads_impl.h"

void mtx_destroy(mtx_t *mtx) {
    __axc_mtx_destroy(mtx);
}

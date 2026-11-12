/* cnd_destroy (C17 7.26.3.2): cond holds no resources to release. */
#include "axc/threads_impl.h"

void cnd_destroy(cnd_t *cond) {
    __axc_cnd_destroy(cond);
}

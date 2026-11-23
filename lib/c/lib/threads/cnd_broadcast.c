/* cnd_broadcast (C17 7.26.3.1): unblocks every thread waiting on cond. */
#include "axc/threads_impl.h"

int cnd_broadcast(cnd_t *cond) {
    return (int)__axc_cnd_broadcast(cond);
}

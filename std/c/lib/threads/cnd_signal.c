/* cnd_signal (C17 7.26.3.4): unblocks one thread waiting on cond, if any. */
#include "axc/threads_impl.h"

int cnd_signal(cnd_t *cond) {
    return (int)__axc_cnd_signal(cond);
}

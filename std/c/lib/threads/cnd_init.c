/* cnd_init (C17 7.26.3.3): a condition variable with no waiters. */
#include "axc/threads_impl.h"

int cnd_init(cnd_t *cond) {
    return (int)__axc_cnd_init(cond);
}

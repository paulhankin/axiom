/* cnd_wait (C17 7.26.3.6): unlocks mtx, blocks until cond is signaled (or
   a spurious wake), locks mtx again. thrd_error if mtx is not locked. */
#include "axc/threads_impl.h"

int cnd_wait(cnd_t *cond, mtx_t *mtx) {
    return (int)__axc_cnd_wait(cond, mtx, -1);
}

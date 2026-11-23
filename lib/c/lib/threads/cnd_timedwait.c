/* cnd_timedwait (C17 7.26.3.5): as cnd_wait, but thrd_timedout once the
   TIME_UTC time *ts has passed (mtx is locked again either way);
   thrd_error for a *ts that is not valid. */
#include "axc/threads_impl.h"

int cnd_timedwait(cnd_t *__AXC_RESTRICT cond, mtx_t *__AXC_RESTRICT mtx, const struct timespec *__AXC_RESTRICT ts) {
    long long t = __axc_ts_ns(ts, 0);
    if (t < 0)
        return thrd_error;
    return (int)__axc_cnd_wait(cond, mtx, t);
}

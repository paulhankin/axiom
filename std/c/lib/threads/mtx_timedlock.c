/* mtx_timedlock (C17 7.26.4.4): as mtx_lock, but thrd_timedout once the
   TIME_UTC time *ts has passed; thrd_error for a *ts that is not valid. */
#include "axc/threads_impl.h"

int mtx_timedlock(mtx_t *__AXC_RESTRICT mtx, const struct timespec *__AXC_RESTRICT ts) {
    long long t = __axc_ts_ns(ts, 0);
    if (t < 0)
        return thrd_error;
    return (int)__axc_mtx_timedlock(mtx, t);
}

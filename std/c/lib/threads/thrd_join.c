/* thrd_join (C17 7.26.5.6): waits for thr to end, then stores its result
   in *res unless res is null; its resources go. thrd_error if thr is
   detached, or is the calling thread. */
#include "axc/threads_impl.h"

int thrd_join(thrd_t thr, int *res) {
    long long r = 0;
    long long e = __axc_thrd_join(thr, &r);
    if (e == thrd_success && res)
        *res = (int)r;
    return (int)e;
}

/* thrd_sleep (C17 7.26.5.7): sleeps for *duration. 0 when it has passed;
   -1 when a signal ended it early (Linux, macOS), with the time left stored
   in *remaining if it is not null; -2 for a duration that is not valid
   (negative, or tv_nsec out of range). */
#include "axc/threads_impl.h"

int thrd_sleep(const struct timespec *duration, struct timespec *remaining) {
    long long left = 0;
    long long ns = __axc_ts_ns(duration, 1);
    long long r;
    if (ns < 0)
        return -2;
    r = __axc_thrd_sleep(ns, &left);
    if (r == -1 && remaining) {
        remaining->tv_sec = (time_t)(left / 1000000000LL);
        remaining->tv_nsec = (long)(left % 1000000000LL);
    }
    return (int)r;
}

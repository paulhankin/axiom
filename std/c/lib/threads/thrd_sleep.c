/* thrd_sleep (C17 7.26.5.7): sleeps for *duration. 0 when it has passed;
   -2 for a duration that is not valid (negative, or tv_nsec out of range).
   No signal ends it early (the runtime resumes the sleep), so it never
   returns -1 and never stores into *remaining. */
#include "axc/threads_impl.h"

int thrd_sleep(const struct timespec *duration, struct timespec *remaining) {
    long long ns = __axc_ts_ns(duration, 1);
    (void)remaining;
    if (ns < 0)
        return -2;
    return (int)__axc_thrd_sleep(ns);
}

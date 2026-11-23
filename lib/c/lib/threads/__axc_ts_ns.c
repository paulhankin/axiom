/* __axc_ts_ns: a struct timespec as nanoseconds (long long, saturated), or
   -1 if it is not a valid time: tv_nsec outside 0..999999999 (C17 7.27.1p4
   gives its range), or with duration set a negative one. A time before
   the epoch is 0 (a deadline that has passed). */
#include "axc/threads_impl.h"

long long __axc_ts_ns(const struct timespec *ts, int duration) {
    const long long max = 9223372036854775807LL;
    long long s = (long long)ts->tv_sec;
    long n = ts->tv_nsec;
    if (n < 0 || n > 999999999L)
        return -1;
    if (s < 0)
        return duration ? -1 : 0;
    if (s > (max - n) / 1000000000LL)
        return max;
    return s * 1000000000LL + n;
}

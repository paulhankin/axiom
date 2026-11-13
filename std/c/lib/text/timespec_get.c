/* timespec_get (C11 7.27.2.5; C23 7.29.2.6): TIME_UTC is the real-time
   clock, and (C23) TIME_MONOTONIC the monotonic one, TIME_ACTIVE the
   processor time of the process (of all its threads), TIME_THREAD_ACTIVE
   that of the calling thread: the runtime's clocks 0 to 3. The seconds are rounded down, the
   nanoseconds 0 to 999999999. 0 for another base. */
#include <axc/text.h>
#include <time.h>

int timespec_get(struct timespec *ts, int base) {
    long long ns, s;
    if (base < 1 || base > 4)
        return 0;
    ns = __axc_clock_ns((unsigned long long)(base - 1));
    s = ns / 1000000000;
    ns %= 1000000000;
    if (ns < 0) {
        ns += 1000000000;
        s--;
    }
    ts->tv_sec = (time_t)s;
    ts->tv_nsec = (long)ns;
    return base;
}

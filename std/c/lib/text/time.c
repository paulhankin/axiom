/* time (C17 7.27.2.4): the seconds of the real-time clock since 1970,
   rounded down. */
#include <axc/text.h>
#include <time.h>

time_t time(time_t *timer) {
    long long ns = __axc_clock_ns(0), s = ns / 1000000000;
    if (ns % 1000000000 < 0)
        s--;
    if (timer)
        *timer = (time_t)s;
    return (time_t)s;
}

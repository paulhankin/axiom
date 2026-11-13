/* timespec_getres (C23 7.29.2.7): the resolution of timespec_get's time
   bases (std/c/rt/text_time.ax); 0 for another base. ts may be null. */
#include <axc/text.h>
#include <time.h>

int timespec_getres(struct timespec *ts, int base) {
    long long ns;
    if (base < 1 || base > 4)
        return 0;
    ns = __axc_clock_res((unsigned long long)(base - 1));
    if (ts) {
        ts->tv_sec = (time_t)(ns / 1000000000);
        ts->tv_nsec = (long)(ns % 1000000000);
    }
    return base;
}

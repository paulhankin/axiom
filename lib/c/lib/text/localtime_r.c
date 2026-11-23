/* localtime_r (C23 7.29.3.6): the broken-down time of *timer in the local
   zone into *buf, tm_isdst as the zone has it; a null pointer if its year
   does not fit tm_year (EOVERFLOW). */
#include <axc/text.h>
#include <errno.h>
#include <time.h>

struct tm *localtime_r(const time_t *timer, struct tm *buf) {
    long long t = *timer;
    long long off, dst;
    __axc_tz_info(t, &off, &dst, 0, 0);
    /* (t + off overflows only far beyond what tm_year holds) */
    if ((off > 0 && t > 0x7fffffffffffffffLL - off) || (off < 0 && t < -0x7fffffffffffffffLL - 1 - off) ||
        __axc_tm_from_seconds(buf, t + off) < 0) {
        errno = EOVERFLOW;
        return 0;
    }
    buf->tm_isdst = (int)dst;
    return buf;
}

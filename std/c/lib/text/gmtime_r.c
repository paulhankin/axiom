/* gmtime_r (C23 7.29.3.4): the broken-down time of *timer in UTC into
   *buf; a null pointer if its year does not fit tm_year (EOVERFLOW). */
#include <axc/text.h>
#include <errno.h>
#include <time.h>

struct tm *gmtime_r(const time_t *timer, struct tm *buf) {
    if (__axc_tm_from_seconds(buf, *timer) < 0) {
        errno = EOVERFLOW;
        return 0;
    }
    buf->tm_isdst = 0;
    return buf;
}

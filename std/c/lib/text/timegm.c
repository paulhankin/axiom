/* timegm (C23 7.29.2.4): as mktime, with the fields a time of UTC
   (tm_isdst is set to 0). */
#include <axc/text.h>
#include <time.h>

time_t timegm(struct tm *timeptr) {
    long long t = __axc_tm_to_seconds(timeptr);
    struct tm r;
    if (__axc_tm_from_seconds(&r, t) < 0)
        return (time_t)-1;
    r.tm_isdst = 0;
    *timeptr = r;
    return (time_t)t;
}

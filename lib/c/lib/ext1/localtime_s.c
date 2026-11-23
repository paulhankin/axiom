/* localtime_s (C11 K.3.8.2.4): the broken-down local time of *timer into
   *result (tm_isdst as the zone has it), which it returns, or a null
   pointer if the year does not fit tm_year (localtime_r's conversion).
   Runtime-constraints: neither pointer null; after a violation the result
   is a null pointer. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <axc/text.h>
#include <time.h>

struct tm *localtime_s(const time_t *__AXC_RESTRICT timer, struct tm *__AXC_RESTRICT result) {
    long long t, off, dst;
    if (timer == NULL || result == NULL) {
        __axc_ext1_violation("localtime_s",
                             timer == NULL ? "timer is a null pointer" : "result is a null pointer", EINVAL);
        return NULL;
    }
    t = *timer;
    __axc_tz_info(t, &off, &dst, 0, 0);
    if ((off > 0 && t > 0x7fffffffffffffffLL - off) || (off < 0 && t < -0x7fffffffffffffffLL - 1 - off) ||
        __axc_tm_from_seconds(result, t + off) < 0)
        return NULL;
    result->tm_isdst = (int)dst;
    return result;
}

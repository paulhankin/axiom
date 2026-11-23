/* gmtime_s (C11 K.3.8.2.3): the broken-down time of *timer in UTC into
   *result, which it returns, or a null pointer if the year does not fit
   tm_year. Runtime-constraints: neither pointer null; after a violation
   the result is a null pointer (no conversion). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <axc/text.h>
#include <time.h>

struct tm *gmtime_s(const time_t *__AXC_RESTRICT timer, struct tm *__AXC_RESTRICT result) {
    if (timer == NULL || result == NULL) {
        __axc_ext1_violation("gmtime_s", timer == NULL ? "timer is a null pointer" : "result is a null pointer",
                             EINVAL);
        return NULL;
    }
    if (__axc_tm_from_seconds(result, *timer) < 0)
        return NULL;
    result->tm_isdst = 0;
    return result;
}

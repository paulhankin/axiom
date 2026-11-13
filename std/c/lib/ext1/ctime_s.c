/* ctime_s (C11 K.3.8.2.2): asctime_s(s, maxsize, localtime_s(timer, &tm)).
   Runtime-constraints: neither pointer null, maxsize 26 to RSIZE_MAX;
   after a violation s[0] is the null character (if s is not null and
   maxsize is 1 to RSIZE_MAX). A time that localtime cannot convert is
   asctime_s's violation (a null timeptr). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <time.h>

errno_t ctime_s(char *s, rsize_t maxsize, const time_t *timer) {
    const char *what = NULL;
    errno_t e = EINVAL;
    struct tm tm;
    if (s == NULL)
        what = "s is a null pointer";
    else if (timer == NULL)
        what = "timer is a null pointer";
    else if (maxsize > RSIZE_MAX)
        what = "maxsize is greater than RSIZE_MAX", e = ERANGE;
    else if (maxsize < 26)
        what = "maxsize is less than 26", e = ERANGE;
    if (what != NULL) {
        if (s != NULL && maxsize > 0 && maxsize <= RSIZE_MAX)
            s[0] = 0;
        __axc_ext1_violation("ctime_s", what, e);
        return e;
    }
    return asctime_s(s, maxsize, localtime_s(timer, &tm));
}

/* asctime_s (C11 K.3.8.2.1): asctime's conversion of *timeptr into s, an
   array of maxsize ("Sun Sep 16 01:03:52 1973\n"). Runtime-constraints:
   neither pointer null, maxsize 26 to RSIZE_MAX, the members of *timeptr
   in their normal ranges (7.27.1p4; tm_sec up to 60) and the year 0 to
   9999; after a violation s[0] is the null character (if s is not null
   and maxsize is 1 to RSIZE_MAX). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <axc/text.h>
#include <string.h>
#include <time.h>

errno_t asctime_s(char *s, rsize_t maxsize, const struct tm *timeptr) {
    const char *what = NULL;
    errno_t e = EINVAL;
    if (s == NULL)
        what = "s is a null pointer";
    else if (timeptr == NULL)
        what = "timeptr is a null pointer";
    else if (maxsize > RSIZE_MAX)
        what = "maxsize is greater than RSIZE_MAX", e = ERANGE;
    else if (maxsize < 26)
        what = "maxsize is less than 26", e = ERANGE;
    else if (timeptr->tm_sec < 0 || timeptr->tm_sec > 60 || timeptr->tm_min < 0 || timeptr->tm_min > 59 ||
             timeptr->tm_hour < 0 || timeptr->tm_hour > 23 || timeptr->tm_mday < 1 || timeptr->tm_mday > 31 ||
             timeptr->tm_mon < 0 || timeptr->tm_mon > 11 || timeptr->tm_wday < 0 || timeptr->tm_wday > 6 ||
             timeptr->tm_yday < 0 || timeptr->tm_yday > 365)
        what = "a member of *timeptr is outside its normal range";
    else if (timeptr->tm_year < -1900 || timeptr->tm_year > 9999 - 1900)
        what = "the year is outside 0 to 9999", e = ERANGE;
    if (what != NULL) {
        if (s != NULL && maxsize > 0 && maxsize <= RSIZE_MAX)
            s[0] = 0;
        __axc_ext1_violation("asctime_s", what, e);
        return e;
    }
    {
        char buf[64];
        __axc_asctime_to(buf, timeptr);
        memcpy(s, buf, strlen(buf) + 1);
    }
    return 0;
}

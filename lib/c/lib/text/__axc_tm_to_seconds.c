/* __axc_tm_to_seconds - the seconds since 1970 of a broken-down time whose
   fields may have any int values (C17 7.27.2.3p2: they are normalized as
   if the excess carried into the next field; tm_wday and tm_yday are
   ignored). Every such time fits in 64 bits. */
#include <axc/text.h>
#include <time.h>

long long __axc_tm_to_seconds(const struct tm *tm) {
    long long y = (long long)tm->tm_year + 1900, mon = tm->tm_mon;
    long long q = mon / 12, r = mon % 12;
    if (r < 0) {
        r += 12;
        q--;
    }
    y += q;
    return (__axc_days_from_civil(y, (int)r + 1, 1) + tm->tm_mday - 1) * 86400 + (long long)tm->tm_hour * 3600 +
           (long long)tm->tm_min * 60 + tm->tm_sec;
}

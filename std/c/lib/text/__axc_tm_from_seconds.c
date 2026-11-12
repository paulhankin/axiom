/* __axc_tm_from_seconds - the broken-down time of a count of seconds since
   1970 (axc/text.h): the date by H. Hinnant's algorithm ("chrono-
   Compatible Low-Level Date Algorithms", 2013), the inverse of
   __axc_days_from_civil; 1970-01-01 was a Thursday. */
#include <axc/text.h>
#include <limits.h>
#include <time.h>

int __axc_tm_from_seconds(struct tm *tm, long long s) {
    long long days = s / 86400, rem = s % 86400, z, era, doe, yoe, y, doy, mp, d, m;
    if (rem < 0) {
        rem += 86400;
        days--;
    }
    z = days + 719468;
    era = (z >= 0 ? z : z - 146096) / 146097;
    doe = z - era * 146097;
    yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = yoe + era * 400;
    doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    if (m <= 2)
        y++;
    if (y - 1900 < INT_MIN || y - 1900 > INT_MAX)
        return -1;
    tm->tm_year = (int)(y - 1900);
    tm->tm_mon = (int)(m - 1);
    tm->tm_mday = (int)d;
    tm->tm_hour = (int)(rem / 3600);
    tm->tm_min = (int)(rem / 60 % 60);
    tm->tm_sec = (int)(rem % 60);
    tm->tm_yday = (int)(days - __axc_days_from_civil(y, 1, 1));
    tm->tm_wday = (int)((days % 7 + 11) % 7);
    return 0;
}

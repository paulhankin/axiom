/* Date and time reports: the broken-down UTC times of instants over four
   centuries (gmtime, timegm), the weekdays, week numbers and days of the
   year of strftime's conversions, a month's calendar, the days between
   dates by difftime, mktime's normalization of out-of-range fields with the
   zone fixed to UTC, and asctime's format. The year 1's %C is "00" (C17
   7.27.3.5: the century as 00-99); another library writes "0". */
#include <stdio.h>
#include <string.h>
#include <time.h>

/* (asctime is deprecated in C23, and used here) */
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static void show(time_t t) {
    char buf[128];
    struct tm *tm = gmtime(&t);
    strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S %a %b %j %U %W %V %G %u %w %C %y %e %I %p", tm);
    printf("%lld: %s\n", (long long)t, buf);
}

int main(void) {
    static const long long instants[] = {0, 951782400, 1234567890, -1, -2208988800LL, 4102444800LL, 253402300799LL,
                                         -62135596800LL, 1709164800, 1735689599};
    size_t i;
    struct tm tm;
    char buf[64];
    for (i = 0; i < sizeof instants / sizeof instants[0]; i++)
        show((time_t)instants[i]);
    /* a calendar of February 2024 */
    memset(&tm, 0, sizeof tm);
    tm.tm_year = 124;
    tm.tm_mon = 1;
    tm.tm_mday = 1;
    {
        time_t first = timegm(&tm);
        int d, wd = tm.tm_wday;
        printf("     February 2024\nSu Mo Tu We Th Fr Sa\n");
        for (d = 0; d < wd; d++)
            printf("   ");
        for (d = 1; d <= 29; d++) {
            printf("%2d%c", d, (wd + d) % 7 == 0 || d == 29 ? '\n' : ' ');
        }
        (void)first;
    }
    /* days between dates */
    {
        struct tm a = {0}, b = {0};
        a.tm_year = 0;
        a.tm_mday = 1;
        b.tm_year = 200;
        b.tm_mday = 1;
        printf("days 1900-2100: %.0f\n", difftime(timegm(&b), timegm(&a)) / 86400);
    }
    /* normalization: 2023-14-35 25:61:61 */
    memset(&tm, 0, sizeof tm);
    tm.tm_year = 123;
    tm.tm_mon = 13;
    tm.tm_mday = 35;
    tm.tm_hour = 25;
    tm.tm_min = 61;
    tm.tm_sec = 61;
    {
        time_t t = timegm(&tm);
        strftime(buf, sizeof buf, "%F %T", &tm);
        printf("normalized: %s yday %d wday %d (%lld)\n", buf, tm.tm_yday, tm.tm_wday, (long long)t);
    }
    {
        time_t t = 1234567890;
        printf("asctime: %s", asctime(gmtime(&t)));
    }
    /* leap years over four centuries */
    {
        int y, leaps = 0;
        for (y = 1600; y < 2000; y++) {
            struct tm f = {0};
            f.tm_year = y - 1900;
            f.tm_mon = 1;
            f.tm_mday = 29;
            timegm(&f);
            leaps += f.tm_mon == 1;
        }
        printf("leap years 1600-1999: %d\n", leaps);
    }
    return 0;
}
/* flags: --c-std=c23 */

// expect: 0: 1970-01-01 00:00:00 Thu Jan 001 00 00 01 1970 4 4 19 70  1 12 AM
// expect: 951782400: 2000-02-29 00:00:00 Tue Feb 060 09 09 09 2000 2 2 20 00 29 12 AM
// expect: 1234567890: 2009-02-13 23:31:30 Fri Feb 044 06 06 07 2009 5 5 20 09 13 11 PM
// expect: -1: 1969-12-31 23:59:59 Wed Dec 365 52 52 01 1970 3 3 19 69 31 11 PM
// expect: -2208988800: 1900-01-01 00:00:00 Mon Jan 001 00 01 01 1900 1 1 19 00  1 12 AM
// expect: 4102444800: 2100-01-01 00:00:00 Fri Jan 001 00 00 53 2099 5 5 21 00  1 12 AM
// expect: 253402300799: 9999-12-31 23:59:59 Fri Dec 365 52 52 52 9999 5 5 99 99 31 11 PM
// expect: -62135596800: 1-01-01 00:00:00 Mon Jan 001 00 01 01 1 1 1 00 01  1 12 AM
// expect: 1709164800: 2024-02-29 00:00:00 Thu Feb 060 08 09 09 2024 4 4 20 24 29 12 AM
// expect: 1735689599: 2024-12-31 23:59:59 Tue Dec 366 52 53 01 2025 2 2 20 24 31 11 PM
// expect:      February 2024
// expect: Su Mo Tu We Th Fr Sa
// expect:              1  2  3
// expect:  4  5  6  7  8  9 10
// expect: 11 12 13 14 15 16 17
// expect: 18 19 20 21 22 23 24
// expect: 25 26 27 28 29
// expect: days 1900-2100: 73049
// expect: normalized: 2024-03-07 02:02:01 yday 66 wday 4 (1709776921)
// expect: asctime: Fri Feb 13 23:31:30 2009
// expect: leap years 1600-1999: 97

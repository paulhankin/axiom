/* Windows: the local time zone is the system's setting when $TZ is not
   set (std/time/tz's windows_local): localtime, mktime and strftime's %Z
   and %z agree with the system's own conversion
   (SystemTimeToTzSpecificLocalTime with the zone's rules for the year,
   GetTimeZoneInformationForYear's, which change from year to year: the
   United States' in 2007) at instants from
   1970 to 2037 and around each change of the offset. The test's
   environment sets TZ for the system (Wine takes its zone from it) and the
   program removes it from its own before the zone is read. */
/* windows-only */
/* env: TZ=America/New_York */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "print.h"

typedef struct {
    unsigned short year, month, dow, day, hour, minute, second, ms;
} SYSTEMTIME;
typedef struct {
    long bias;
    unsigned short std_name[32];
    SYSTEMTIME std_date;
    long std_bias;
    unsigned short dst_name[32];
    SYSTEMTIME dst_date;
    long dst_bias;
    unsigned short key[128];
    unsigned char disabled;
} DYNAMIC_TIME_ZONE_INFORMATION;

/* kernel32.dll */
int SetEnvironmentVariableW(const unsigned short *name, const unsigned short *value);
unsigned long GetDynamicTimeZoneInformation(DYNAMIC_TIME_ZONE_INFORMATION *info);
int GetTimeZoneInformationForYear(unsigned short year, const DYNAMIC_TIME_ZONE_INFORMATION *dynamic, void *info);
int SystemTimeToTzSpecificLocalTime(const void *info, const SYSTEMTIME *utc, SYSTEMTIME *local);

static DYNAMIC_TIME_ZONE_INFORMATION zone;
static long checked, bad, changes;

/* the days since 1970 of a civil date (proleptic Gregorian) */
static long long days(long long y, int m, int d) {
    y -= m <= 2;
    long long era = (y >= 0 ? y : y - 399) / 400;
    long long yoe = y - era * 400;
    long long doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    return era * 146097 + yoe * 365 + yoe / 4 - yoe / 100 + doy - 719468;
}

/* the system's offset (seconds east) at t */
static long long system_offset(long long t) {
    SYSTEMTIME u, l;
    long long d = t / 86400, s = t % 86400;
    long long y = 1970;
    while (days(y + 1, 1, 1) <= d) y++;
    int m = 1;
    while (m < 12 && days(y, m + 1, 1) <= d) m++;
    memset(&u, 0, sizeof u);
    u.year = (unsigned short)y;
    u.month = (unsigned short)m;
    u.day = (unsigned short)(d - days(y, m, 1) + 1);
    u.hour = (unsigned short)(s / 3600);
    u.minute = (unsigned short)(s / 60 % 60);
    u.second = (unsigned short)(s % 60);
    /* (TIME_ZONE_INFORMATION: the dynamic one's first 172 bytes) */
    static unsigned char year_info[172];
    static long long year_of = -1;
    if (y != year_of && !GetTimeZoneInformationForYear((unsigned short)y, &zone, year_info)) return -999999;
    year_of = y;
    if (!SystemTimeToTzSpecificLocalTime(year_info, &u, &l)) return -999999;
    long long lt = days(l.year, l.month, l.day) * 86400 + l.hour * 3600 + l.minute * 60 + l.second;
    return lt - t;
}

static long long prev_off = -1;

static void check(time_t t) {
    struct tm tm = *localtime(&t);
    long long off = system_offset((long long)t);
    long long lt = days(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday) * 86400 + tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
    char z[8], name[64];
    strftime(z, sizeof z, "%z", &tm);
    strftime(name, sizeof name, "%Z", &tm);
    long zoff = (z[0] == '-' ? -1 : 1) * ((z[1] - '0') * 36000L + (z[2] - '0') * 3600L + (z[3] - '0') * 600L + (z[4] - '0') * 60L);
    struct tm back = tm;
    int ok = lt - (long long)t == off && zoff == off && mktime(&back) == t &&
             (tm.tm_isdst > 0) == (off != -60LL * (zone.bias + zone.std_bias)) &&
             strcmp(name, tm.tm_isdst > 0 ? "Eastern Daylight Time" : "Eastern Standard Time") == 0;
    if (off != prev_off) changes++;
    prev_off = off;
    checked++;
    if (!ok && bad++ < 5) printf("at %lld: %04d-%02d-%02d %02d:%02d:%02d %s %s dst %d, the system's offset %lld\n", (long long)t,
                                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, z, name,
                                 tm.tm_isdst, off);
}

int main(void) {
    static const unsigned short tz[] = {'T', 'Z', 0};
    SetEnvironmentVariableW(tz, 0);
    if (GetDynamicTimeZoneInformation(&zone) == 0xffffffffu) return 1;
    /* every 3 hours (hence near every change) from 1970 to 2037 */
    for (long long t = 0; t < 2145916800LL; t += 3 * 3600 + 7)
        check((time_t)t);
    print_bool(checked > 190000); /* expect: true */
    print_bool(changes > 100);    /* expect: true */
    print_i64(bad);               /* expect: 0 */
    return 0;
}

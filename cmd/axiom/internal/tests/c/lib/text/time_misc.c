/* <time.h> beside the calendar: strftime's size limit (the null character
   counted), a conversion specifier it does not know, the E and O
   modifiers on specifiers they do not apply to, fields out of their
   ranges, %z and %Z (none for tm_isdst < 0, else the local zone's); the
   same through wcsftime; asctime and ctime; difftime exactly at the ends
   of time_t; time, clock, timespec_get and timespec_getres (C23) of each
   time base. */
/* flags: --c-std=c23 */
#include <time.h>
#include <wchar.h>
#include "../../mix/print.h"
#include <stdlib.h>
#include <string.h>

static void wide(const wchar_t *w) {
    char b[128];
    int i;
    for (i = 0; w[i] && i < 127; i++)
        b[i] = w[i] < 128 ? (char)w[i] : '?';
    b[i] = 0;
    print_cstr(b);
}

int main(void) {
    char buf[64];
    wchar_t wbuf[64];
    struct tm tm = {0}, odd;
    struct timespec a, b, r;
    time_t t = 0, big = 0x7fffffffffffffffLL, small = -0x7fffffffffffffffLL - 1;
    long long i, spin = 0;
    clock_t c0;
    tm.tm_year = 99;
    tm.tm_mon = 11;
    tm.tm_mday = 31;
    tm.tm_hour = 23;
    tm.tm_min = 59;
    tm.tm_sec = 60;
    tm.tm_wday = 5;
    tm.tm_yday = 364;
    tm.tm_isdst = -1;
    /* the size: 10 characters need 11 bytes */
    print_i64((long long)strftime(buf, 11, "%Y-%m-%d", &tm));
    print_cstr(buf);
    print_i64((long long)strftime(buf, 10, "%Y-%m-%d", &tm));
    print_i64((long long)strftime(buf, 1, "", &tm));
    print_bool(buf[0] == 0);
    print_i64((long long)strftime(buf, 0, "", &tm));
    /* not conversions: copied */
    strftime(buf, sizeof buf, "%q|%Ea|%Od|%OY|%E|%", &tm);
    print_cstr(buf);
    /* a leap second; no time zone */
    strftime(buf, sizeof buf, "%T [%z] [%Z] %p %I", &tm);
    print_cstr(buf);
    tm.tm_isdst = 0;
    strftime(buf, sizeof buf, "%z", &tm);
    print_bool(strlen(buf) == 5 && (buf[0] == '+' || buf[0] == '-'));
    strftime(buf, sizeof buf, "%Z", &tm);
    print_bool(buf[0] != 0);
    /* fields out of range: converted all the same, names as "?" */
    odd = tm;
    odd.tm_wday = 9;
    odd.tm_mon = -1;
    odd.tm_mday = 123;
    odd.tm_hour = 0;
    strftime(buf, sizeof buf, "%a %A %b %B %d %e %I %p", &odd);
    print_cstr(buf);
    /* wcsftime */
    tm.tm_isdst = -1;
    print_i64((long long)wcsftime(wbuf, 64, L"%A %d %B %Y, %H:%M:%S é %% %q [%z]", &tm));
    wide(wbuf);
    print_i64((long long)wcsftime(wbuf, 4, L"%Y", &tm));
    print_i64((long long)wcsftime(wbuf, 5, L"%Y", &tm));
    /* asctime: the format, and fields beyond its 26 bytes */
    print_str(asctime(&tm), 24);
    odd = tm;
    odd.tm_year = 2147483647;
    odd.tm_mday = -5;
    odd.tm_hour = -1;
    odd.tm_mon = 12;
    print_str(asctime(&odd), strlen(asctime(&odd)) - 1);
    t = 0;
    print_bool(ctime(&t) != 0 && strlen(ctime(&t)) == 25);
    print_bool(ctime(&big) == 0);
    /* difftime */
    print_f64(difftime(5, 7));
    print_f64(difftime(big, small));
    print_f64(difftime(small, big));
    print_f64(difftime(9007199254740993LL, 0));
    /* the clocks */
    t = time(0);
    print_bool(t > 1700000000 && time(&big) == big && big >= t);
    print_i64(timespec_get(&a, TIME_UTC));
    print_bool(a.tv_sec >= t && a.tv_sec - t < 100 && a.tv_nsec >= 0 && a.tv_nsec < 1000000000);
    print_i64(timespec_get(&a, TIME_MONOTONIC));
    print_i64(timespec_get(&b, TIME_MONOTONIC));
    print_bool(b.tv_sec > a.tv_sec || (b.tv_sec == a.tv_sec && b.tv_nsec >= a.tv_nsec));
    print_i64(timespec_get(&a, TIME_ACTIVE));
    print_i64(timespec_get(&a, 0) + timespec_get(&a, 4));
    print_bool(timespec_getres(&r, TIME_UTC) == TIME_UTC && r.tv_sec == 0 && r.tv_nsec > 0);
    print_bool(timespec_getres(0, TIME_MONOTONIC) == TIME_MONOTONIC && timespec_getres(&r, TIME_ACTIVE) &&
               r.tv_nsec > 0 && timespec_getres(&r, 7) == 0);
    c0 = clock();
    for (i = 0; i < 20000000; i++)
        spin += i ^ (spin >> 3);
    print_bool(c0 >= 0 && clock() > c0 && spin != 1 && CLOCKS_PER_SEC == 1000000);
    return 0;
    /* expect: 10 */
    /* expect: 1999-12-31 */
    /* expect: 0 */
    /* expect: 0 */
    /* expect: true */
    /* expect: 0 */
    /* expect: %q|%Ea|31|%OY|%E|% */
    /* expect: 23:59:60 [] [] PM 11 */
    /* expect: true */
    /* expect: true */
    /* expect: ? ? ? ? 123 123 12 AM */
    /* expect: 43 */
    /* expect: Friday 31 December 1999, 23:59:60 ? % %q [] */
    /* expect: 0 */
    /* expect: 4 */
    /* expect: Fri Dec 31 23:59:60 1999 */
    /* expect: Fri ??? -5 -01:59:60 2147485547 */
    /* expect: true */
    /* expect: true */
    /* expect: -2 */
    /* expect: 18446744073709552000 */
    /* expect: -18446744073709552000 */
    /* expect: 9007199254740992 */
    /* expect: true */
    /* expect: 1 */
    /* expect: true */
    /* expect: 2 */
    /* expect: 2 */
    /* expect: true */
    /* expect: 3 */
    /* expect: 0 */
    /* expect: true */
    /* expect: true */
    /* expect: true */
}

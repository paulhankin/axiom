/* Annex K's <time.h> (C11 K.3.8): asctime_s, ctime_s, gmtime_s and
   localtime_s, their buffers and runtime-constraints (normalized members,
   the years 0 to 9999, at least 26 characters). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static void handler(const char *restrict msg, void *restrict ptr, errno_t error) {
    (void)ptr;
    printf("handler: %s (%d)\n", msg, error != 0);
}

int main(void) {
    char s[32];
    struct tm tm, *p;
    time_t t = 1234567890, huge = (time_t)0x7fffffffffffffffLL;
    errno_t e;
    set_constraint_handler_s(handler);
    p = gmtime_s(&t, &tm);
    printf("gmtime_s %d %d-%02d-%02d %02d:%02d:%02d %d %d\n", p == &tm, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
           tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_wday, tm.tm_yday);
    e = asctime_s(s, sizeof s, &tm);
    printf("asctime_s %d %s", e, s);
    e = asctime_s(s, 25, &tm);
    printf("asctime_s 25 %d [%s]\n", e != 0, s);
    tm.tm_mon = 12;
    e = asctime_s(s, sizeof s, &tm);
    tm.tm_mon = 1;
    tm.tm_year = 10000 - 1900;
    e = asctime_s(s, sizeof s, &tm);
    tm.tm_year = -1900;
    e = asctime_s(s, sizeof s, &tm);
    printf("year 0 %d %s", e, s);
    e = asctime_s(s, sizeof s, NULL);
    /* (the local zone: UTC in the suite, TZ unset or empty) */
    p = localtime_s(&t, &tm);
    printf("localtime_s %d\n", p == &tm);
    e = ctime_s(s, sizeof s, &t);
    printf("ctime_s %d %d\n", e, s[24] == '\n');
    e = ctime_s(s, 10, &t);
    printf("ctime_s small %d [%s]\n", e != 0, s);
    printf("gmtime_s huge %d\n", gmtime_s(&huge, &tm) == NULL);
    p = gmtime_s(NULL, &tm);
    p = localtime_s(&t, NULL);
    printf("null %d\n", p == NULL);
    return 0;
}

// expect: gmtime_s 1 2009-02-13 23:31:30 5 43
// expect: asctime_s 0 Fri Feb 13 23:31:30 2009
// expect: handler: asctime_s: maxsize is less than 26 (1)
// expect: asctime_s 25 1 []
// expect: handler: asctime_s: a member of *timeptr is outside its normal range (1)
// expect: handler: asctime_s: the year is outside 0 to 9999 (1)
// expect: year 0 0 Fri Feb 13 23:31:30 0
// expect: handler: asctime_s: timeptr is a null pointer (1)
// expect: localtime_s 1
// expect: ctime_s 0 1
// expect: handler: ctime_s: maxsize is less than 26 (1)
// expect: ctime_s small 1 []
// expect: gmtime_s huge 1
// expect: handler: gmtime_s: timer is a null pointer (1)
// expect: handler: localtime_s: result is a null pointer (1)
// expect: null 1

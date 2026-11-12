/* time.h - date and time (C89 4.12, C99 7.23, C11 7.27, C23 7.29), for the
   Axiom compiler's C library. time_t counts seconds since 1970-01-01
   00:00:00 UTC (leap seconds ignored), in 64 bits; the calendar is the
   proleptic Gregorian one, of any year an int holds. Local time follows
   the system's time zone (std/time/tz: $TZ, else /etc/localtime; UTC where
   there is none, as on Windows and WebAssembly), read once
   (docs/c-text.md, "Time"). */
#ifndef __AXIOM_TIME_H
#define __AXIOM_TIME_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_TIME_H__ 202311L
#endif

#define __AXIOM_WANT_SIZE_T
#include <axc/types.h>

#define NULL ((void *)0)

/* (time_t and clock_t: the 64-bit type of int64_t) */
#ifdef __AXIOM_INT64_TYPE__
#define __AXIOM_TIME_TYPE __AXIOM_INT64_TYPE__
#elif __AXIOM_LONG_WIDTH__ == 64
#define __AXIOM_TIME_TYPE long
#elif __STDC_VERSION__ >= 199901L
#define __AXIOM_TIME_TYPE long long
#else
#define __AXIOM_TIME_TYPE __axiom_int64
#endif
typedef __AXIOM_TIME_TYPE time_t;
typedef __AXIOM_TIME_TYPE clock_t;
#undef __AXIOM_TIME_TYPE

/* clock's unit: microseconds */
#define CLOCKS_PER_SEC ((clock_t)1000000)

struct tm {
    int tm_sec;   /* 0 to 60 */
    int tm_min;   /* 0 to 59 */
    int tm_hour;  /* 0 to 23 */
    int tm_mday;  /* 1 to 31 */
    int tm_mon;   /* 0 to 11 */
    int tm_year;  /* years since 1900 */
    int tm_wday;  /* days since Sunday: 0 to 6 */
    int tm_yday;  /* days since January 1: 0 to 365 */
    int tm_isdst; /* > 0 daylight saving time, 0 not, < 0 not known */
};

#if __STDC_VERSION__ >= 201112L
struct timespec {
    time_t tv_sec;
    long tv_nsec;
};
/* the time bases of timespec_get (C11 7.27.2.5; C23 7.29.1p5) */
#define TIME_UTC 1
#if __STDC_VERSION__ >= 202311L
#define TIME_MONOTONIC 2
#define TIME_ACTIVE 3
#endif
#endif

/* time manipulation (7.27.2) */
clock_t clock(void);
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm *timeptr);
time_t time(time_t *timer);
#if __STDC_VERSION__ >= 201112L
int timespec_get(struct timespec *ts, int base);
#endif
#if __STDC_VERSION__ >= 202311L
time_t timegm(struct tm *timeptr);
int timespec_getres(struct timespec *ts, int base);
#endif

/* time conversion (7.27.3) */
#if __STDC_VERSION__ >= 202311L
[[deprecated]] char *asctime(const struct tm *timeptr);
[[deprecated]] char *ctime(const time_t *timer);
#else
char *asctime(const struct tm *timeptr);
char *ctime(const time_t *timer);
#endif
struct tm *gmtime(const time_t *timer);
struct tm *localtime(const time_t *timer);
#if __STDC_VERSION__ >= 202311L
struct tm *gmtime_r(const time_t *timer, struct tm *buf);
struct tm *localtime_r(const time_t *timer, struct tm *buf);
#endif
#if __STDC_VERSION__ >= 199901L
size_t strftime(char *restrict s, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);
#else
size_t strftime(char *s, size_t maxsize, const char *format, const struct tm *timeptr);
#endif

#endif

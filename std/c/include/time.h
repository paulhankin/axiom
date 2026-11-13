/* time.h - date and time (C89 4.12, C99 7.23, C11 7.27, C23 7.29), for the
   Axiom compiler's C library. time_t counts seconds since 1970-01-01
   00:00:00 UTC (leap seconds ignored), in 64 bits; the calendar is the
   proleptic Gregorian one, of any year an int holds. Local time follows
   the system's time zone (std/time/tz: $TZ, else /etc/localtime; UTC where
   there is none, as on Windows and WebAssembly), read once
   (docs/c-text.md, "Time"). */
/* (Annex K: whether its names are asked for, K.3.1.1) */
#include <axc/ext1.h>
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
#define TIME_THREAD_ACTIVE 4
#endif
#endif

/* time manipulation (7.27.2) */
clock_t clock(void);
double difftime(time_t __time1, time_t __time0);
time_t mktime(struct tm *__timeptr);
time_t time(time_t *__timer);
#if __STDC_VERSION__ >= 201112L
int timespec_get(struct timespec *__ts, int __base);
#endif
#if __STDC_VERSION__ >= 202311L
time_t timegm(struct tm *__timeptr);
int timespec_getres(struct timespec *__ts, int __base);
#endif

/* time conversion (7.27.3) */
#if __STDC_VERSION__ >= 202311L
[[deprecated]] char *asctime(const struct tm *__timeptr);
[[deprecated]] char *ctime(const time_t *__timer);
#else
char *asctime(const struct tm *__timeptr);
char *ctime(const time_t *__timer);
#endif
struct tm *gmtime(const time_t *__timer);
struct tm *localtime(const time_t *__timer);
#if __STDC_VERSION__ >= 202311L
struct tm *gmtime_r(const time_t *__timer, struct tm *__buf);
struct tm *localtime_r(const time_t *__timer, struct tm *__buf);
#endif
#if __STDC_VERSION__ >= 199901L
size_t strftime(char *restrict __s, size_t __maxsize, const char *restrict __format, const struct tm *restrict __timeptr)
    __axiom_attribute__((__format__(strftime, 3, 0)));
#else
size_t strftime(char *__s, size_t __maxsize, const char *__format, const struct tm *__timeptr)
    __axiom_attribute__((__format__(strftime, 3, 0)));
#endif

#if __AXC_EXT1
/* Annex K (K.3.8): the bounds-checking interfaces (std/c/lib/ext1/) */
#define __AXIOM_WANT_ERRNO_T
#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_RSIZE_T
#include <axc/types.h>
errno_t asctime_s(char *__s, rsize_t __maxsize, const struct tm *__timeptr);
errno_t ctime_s(char *__s, rsize_t __maxsize, const time_t *__timer);
struct tm *gmtime_s(const time_t *__AXC_RESTRICT __timer, struct tm *__AXC_RESTRICT __result);
struct tm *localtime_s(const time_t *__AXC_RESTRICT __timer, struct tm *__AXC_RESTRICT __result);
#endif

#endif

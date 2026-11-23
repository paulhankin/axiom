/* __axc_iso_week - the ISO 8601 week-based year and week of a broken-down
   time, from tm_year, tm_wday and tm_yday (C17 7.27.3.5p4): weeks begin on
   Monday, and week 1 is the one with January 4 in it (with the year's
   first Thursday), so that a year has 52 or 53 weeks and its first days
   may be in the last week of the year before, its last days in week 1 of
   the next. */
#include <axc/text.h>
#include <time.h>

static int leap(long long y) {
    return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}

/* the weeks of a year whose January 1 is on weekday jan1 (Monday 0): 53 if
   it begins on a Thursday, or on a Wednesday in a leap year */
static int weeks(long long y, int jan1) {
    return jan1 == 3 || (jan1 == 2 && leap(y)) ? 53 : 52;
}

long long __axc_iso_week(const struct tm *tm, int *week) {
    long long y = (long long)tm->tm_year + 1900;
    int yday = tm->tm_yday, wd = ((tm->tm_wday % 7) + 13) % 7; /* (Monday 0) */
    int jan1 = ((wd - yday) % 7 + 7) % 7;
    int w = (yday - wd + 10) / 7;
    if (yday - wd + 10 < 7) {
        /* (the last week of the year before, whose January 1 is 365 or 366
           days before this one's) */
        int prev = ((jan1 - (leap(y - 1) ? 366 : 365)) % 7 + 7) % 7;
        *week = weeks(y - 1, prev);
        return y - 1;
    }
    if (w > weeks(y, jan1)) {
        *week = 1;
        return y + 1;
    }
    *week = w;
    return y;
}

/* The local time zone follows $TZ while the program runs: a conversion
   whose $TZ is not the one the zone was read with reads it again (as
   POSIX's tzset does), so localtime, mktime and strftime's %Z see the
   new zone. The environment is changed through std/os (setenv.ax). */
/* uses: setenv.ax */
#include <stdio.h>
#include <time.h>

long long test_setenv(const char *name, const char *value);

static void show(const char *tz) {
    time_t t = 1700000000;
    struct tm *tm, copy;
    char buf[64];
    test_setenv("TZ", tz);
    tm = localtime(&t);
    copy = *tm;
    strftime(buf, sizeof buf, "%H:%M %Z %z", tm);
    printf("%s: %s %d %lld\n", tz != NULL ? tz : "(unset)", buf, tm->tm_isdst, (long long)(mktime(&copy) - t));
}

int main(void) {
    show("UTC0");
    show("EST5");
    show("JST-9");
    show("NZST-12NZDT,M9.5.0,M4.1.0/3");
    show("UTC0");
    return 0;
}
// expect: UTC0: 22:13 UTC +0000 0 0
// expect: EST5: 17:13 EST -0500 0 0
// expect: JST-9: 07:13 JST +0900 0 0
// expect: NZST-12NZDT,M9.5.0,M4.1.0/3: 11:13 NZDT +1300 1 0
// expect: UTC0: 22:13 UTC +0000 0 0

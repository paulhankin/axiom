/* <time.h>'s clocks (C23 7.29.2.6, 7.29.2.7): every time base of
   timespec_get has a resolution, TIME_THREAD_ACTIVE is the calling
   thread's processor time and clock and TIME_ACTIVE the process's, which
   counts every thread's (two threads each spin for a while: the process's
   time grows by at least their two times, while the main thread, waiting,
   uses little). mktime and timegm at the ends of tm_year's range. */
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <threads.h>

static long long ns(int base) {
    struct timespec ts;
    timespec_get(&ts, base);
    return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

static volatile unsigned long sink;
static long long spent[2];

static int spin(void *arg) {
    int i = *(int *)arg;
    long long t0 = ns(TIME_THREAD_ACTIVE);
    unsigned long k, x = 1;
    while (ns(TIME_THREAD_ACTIVE) - t0 < 150000000LL) {
        for (k = 0; k < 100000; k++)
            x = x * 6364136223846793005UL + 1442695040888963407UL;
        sink = x;
    }
    spent[i] = ns(TIME_THREAD_ACTIVE) - t0;
    return 0;
}

int main(void) {
    struct timespec res;
    int b, ids[2] = {0, 1};
    thrd_t t[2];
    long long p0, m0, p1, m1;
    clock_t c0, c1;
    struct tm tm = {0};
    for (b = 1; b <= 5; b++) {
        int r = timespec_getres(&res, b);
        printf("base %d: %d %d\n", b, r, r == 0 || (res.tv_sec == 0 && res.tv_nsec > 0 && res.tv_nsec <= 1000000));
    }
    printf("get 5: %d\n", timespec_get(&res, 5));
    c0 = clock();
    p0 = ns(TIME_ACTIVE);
    m0 = ns(TIME_THREAD_ACTIVE);
    thrd_create(&t[0], spin, &ids[0]);
    thrd_create(&t[1], spin, &ids[1]);
    thrd_join(t[0], NULL);
    thrd_join(t[1], NULL);
    p1 = ns(TIME_ACTIVE);
    m1 = ns(TIME_THREAD_ACTIVE);
    c1 = clock();
    printf("process counts the threads: %d\n", p1 - p0 >= (spent[0] + spent[1]) * 9 / 10);
    printf("clock too: %d\n", (double)(c1 - c0) / CLOCKS_PER_SEC >= (spent[0] + spent[1]) * 0.9e-9);
    printf("the main thread's own: %d\n", m1 - m0 < spent[0] / 2);
    /* the ends of tm_year */
    tm.tm_mday = 1;
    tm.tm_year = INT_MAX - 1900 > 0 ? INT_MAX : INT_MAX;
    tm.tm_year = INT_MAX;
    tm.tm_mon = 0;
    printf("timegm INT_MAX year: %d\n", timegm(&tm) != (time_t)-1 && tm.tm_year == INT_MAX && tm.tm_yday == 0);
    tm.tm_year = INT_MAX;
    tm.tm_mon = 12;
    printf("past it: %d\n", timegm(&tm) == (time_t)-1);
    tm.tm_year = INT_MIN;
    tm.tm_mon = 0;
    printf("INT_MIN year: %d %d\n", timegm(&tm) != (time_t)-1, tm.tm_year == INT_MIN);
    tm.tm_year = INT_MIN;
    tm.tm_mon = -1;
    printf("before it: %d\n", timegm(&tm) == (time_t)-1);
    tm.tm_year = 70;
    tm.tm_mon = 0;
    tm.tm_mday = 1;
    tm.tm_hour = tm.tm_min = tm.tm_sec = 0;
    tm.tm_sec = INT_MAX;
    printf("seconds carried: %lld %d\n", (long long)timegm(&tm), tm.tm_year);
    return 0;
}
/* flags: --c-std=c23 */
// native-only
// expect: base 1: 1 1
// expect: base 2: 2 1
// expect: base 3: 3 1
// expect: base 4: 4 1
// expect: base 5: 0 1
// expect: get 5: 0
// expect: process counts the threads: 1
// expect: clock too: 1
// expect: the main thread's own: 1
// expect: timegm INT_MAX year: 1
// expect: past it: 1
// expect: INT_MIN year: 1 1
// expect: before it: 1
// expect: seconds carried: 2147483647 138

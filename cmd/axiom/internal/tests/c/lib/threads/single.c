/* <threads.h> in a program that starts no thread (which on WebAssembly
   then has no shared memory, where a wait cannot be one of the processor:
   std/c/rt/threads.ax sleeps instead): a timed wait times out, a timed
   lock of a mutex the thread holds (not recursive) times out, a sleep
   sleeps. The return from main runs no tss destructor (exit does not:
   7.26.6.1), but the atexit functions. (No thrd_exit, which would link
   the creation of threads: tss.c has it on the main thread.) */
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <time.h>

static void dtor(void *p) {
    printf("dtor %s\n", (char *)p);
}

static void at_end(void) {
    printf("atexit\n");
}

static long long ms_since(const struct timespec *a) {
    struct timespec b;
    timespec_get(&b, TIME_UTC);
    return (b.tv_sec - a->tv_sec) * 1000LL + (b.tv_nsec - a->tv_nsec) / 1000000;
}

int main(void) {
    mtx_t m;
    cnd_t c;
    tss_t k;
    struct timespec start, ts, d = { 0, 30000000 };
    mtx_init(&m, mtx_timed);
    cnd_init(&c);
    mtx_lock(&m);
    timespec_get(&start, TIME_UTC);
    ts = start;
    ts.tv_nsec += 30000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec++;
    }
    printf("timedwait %d", cnd_timedwait(&c, &m, &ts));
    printf(" %d\n", ms_since(&start) >= 29);
    timespec_get(&start, TIME_UTC);
    ts = start;
    ts.tv_sec += 0;
    ts.tv_nsec += 30000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec++;
    }
    printf("timedlock %d", mtx_timedlock(&m, &ts));
    printf(" %d\n", ms_since(&start) >= 29);
    mtx_unlock(&m);
    timespec_get(&start, TIME_UTC);
    printf("sleep %d", thrd_sleep(&d, NULL));
    printf(" %d\n", ms_since(&start) >= 29);
    tss_create(&k, dtor);
    tss_set(k, "main");
    atexit(at_end);
    return 0;
}
/* expect: timedwait 4 1 */
/* expect: timedlock 4 1 */
/* expect: sleep 0 1 */
/* expect: atexit */

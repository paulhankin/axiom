/* Mutexes under contention (C17 7.26.4): 64 threads add to counters under
   a plain mutex, a timed one taken with mtx_timedlock, and a recursive one
   taken three times deep; the plain counters would come out short without
   mutual exclusion (the read and the write are apart). */
#include <stdio.h>
#include <threads.h>
#include <time.h>

#define THREADS 64
#define ROUNDS 2000

static mtx_t plain, timed, rec;
static volatile long a, b, c;
static int timeouts;

static int work(void *arg) {
    int i;
    struct timespec far;
    (void)arg;
    timespec_get(&far, TIME_UTC);
    far.tv_sec += 60;
    for (i = 0; i < ROUNDS; i++) {
        long x;
        mtx_lock(&plain);
        x = a;
        thrd_yield();
        a = x + 1;
        mtx_unlock(&plain);
        if (mtx_timedlock(&timed, &far) != thrd_success) {
            timeouts++;
            continue;
        }
        x = b;
        b = x + 2;
        mtx_unlock(&timed);
        mtx_lock(&rec);
        mtx_lock(&rec);
        if (mtx_trylock(&rec) == thrd_success) {
            x = c;
            c = x + 3;
            mtx_unlock(&rec);
        }
        mtx_unlock(&rec);
        mtx_unlock(&rec);
    }
    return 0;
}

int main(void) {
    thrd_t t[THREADS];
    int i, started = 0;
    mtx_init(&plain, mtx_plain);
    mtx_init(&timed, mtx_timed);
    mtx_init(&rec, mtx_plain | mtx_recursive);
    for (i = 0; i < THREADS; i++)
        started += thrd_create(&t[i], work, NULL) == thrd_success;
    for (i = 0; i < started; i++)
        thrd_join(t[i], NULL);
    printf("%d %ld %ld %ld %d\n", started, a, b, c, timeouts);
    mtx_destroy(&plain);
    mtx_destroy(&timed);
    mtx_destroy(&rec);
    return 0;
}
/* expect: 64 128000 256000 384000 0 */

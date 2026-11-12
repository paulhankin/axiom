/* Condition variables (C17 7.26.3): producers and consumers on a bounded
   buffer, waiting with cnd_timedwait against TIME_UTC deadlines (a
   timeout only makes them look again) and cnd_wait; every item is taken
   once. Then a broadcast wakes every waiter at once. */
#include <stdio.h>
#include <threads.h>
#include <time.h>

#define PRODUCERS 4
#define CONSUMERS 4
#define ITEMS 5000 /* per producer */
#define CAP 8

static mtx_t m;
static cnd_t not_full, not_empty, go;
static int buf[CAP], head, count, done_producers;
static long long sum, taken;
static int started, released;

static struct timespec after_ms(long ms) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    ts.tv_nsec += ms * 1000000L;
    ts.tv_sec += ts.tv_nsec / 1000000000L;
    ts.tv_nsec %= 1000000000L;
    return ts;
}

static int producer(void *arg) {
    int id = (int)(long)arg, i;
    for (i = 1; i <= ITEMS; i++) {
        mtx_lock(&m);
        while (count == CAP) {
            struct timespec ts = after_ms(5);
            int r = cnd_timedwait(&not_full, &m, &ts);
            if (r != thrd_success && r != thrd_timedout)
                return 1;
        }
        buf[(head + count) % CAP] = id * 100000 + i;
        count++;
        cnd_signal(&not_empty);
        mtx_unlock(&m);
    }
    mtx_lock(&m);
    done_producers++;
    cnd_broadcast(&not_empty);
    mtx_unlock(&m);
    return 0;
}

static int consumer(void *arg) {
    (void)arg;
    mtx_lock(&m);
    for (;;) {
        while (count == 0 && done_producers < PRODUCERS) {
            if (taken % 2) {
                cnd_wait(&not_empty, &m);
            } else {
                struct timespec ts = after_ms(3);
                cnd_timedwait(&not_empty, &m, &ts);
            }
        }
        if (count == 0)
            break;
        sum += buf[head] % 100000;
        head = (head + 1) % CAP;
        count--;
        taken++;
        cnd_signal(&not_full);
    }
    mtx_unlock(&m);
    return 0;
}

static int waiter(void *arg) {
    (void)arg;
    mtx_lock(&m);
    started++;
    cnd_signal(&not_empty);
    while (!released)
        cnd_wait(&go, &m);
    started--;
    mtx_unlock(&m);
    return 0;
}

int main(void) {
    thrd_t p[PRODUCERS], c[CONSUMERS], w[16];
    int i;
    mtx_init(&m, mtx_plain);
    cnd_init(&not_full);
    cnd_init(&not_empty);
    cnd_init(&go);
    for (i = 0; i < CONSUMERS; i++)
        thrd_create(&c[i], consumer, NULL);
    for (i = 0; i < PRODUCERS; i++)
        thrd_create(&p[i], producer, (void *)(long)i);
    for (i = 0; i < PRODUCERS; i++)
        thrd_join(p[i], NULL);
    for (i = 0; i < CONSUMERS; i++)
        thrd_join(c[i], NULL);
    printf("%lld %lld %d\n", taken, sum, count);

    for (i = 0; i < 16; i++)
        thrd_create(&w[i], waiter, NULL);
    mtx_lock(&m);
    while (started < 16)
        cnd_wait(&not_empty, &m);
    released = 1;
    cnd_broadcast(&go);
    mtx_unlock(&m);
    for (i = 0; i < 16; i++)
        thrd_join(w[i], NULL);
    printf("%d\n", started);
    return 0;
}
/* expect: 20000 50010000 0 */
/* expect: 0 */

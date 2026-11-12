/* exit from a thread other than the main one (C17 7.22.4.4): the atexit
   functions run once (on that thread), the streams are flushed, and the
   process ends with its status while the main thread waits in thrd_join
   and other threads run. A second thread's exit meanwhile waits. */
/* expect-status: 7 */
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static atomic_int calls;

static void at_end(void) {
    printf("atexit %d\n", atomic_fetch_add(&calls, 1));
    {
        struct timespec ts = { 0, 30000000 };
        thrd_sleep(&ts, NULL);
    }
}

static int spin(void *arg) {
    (void)arg;
    for (;;)
        thrd_yield();
}

static int leaver(void *arg) {
    struct timespec ts = { 0, 10000000 };
    thrd_sleep(&ts, NULL);
    printf("thread %d exits\n", (int)(long)arg);
    exit((int)(long)arg);
}

/* exits while the other's atexit function runs: waits */
static int second(void *arg) {
    (void)arg;
    while (atomic_load(&calls) == 0)
        thrd_yield();
    exit(8);
}

int main(void) {
    thrd_t t, u, v;
    atexit(at_end);
    thrd_create(&u, spin, NULL);
    thrd_create(&t, leaver, (void *)7L);
    thrd_create(&v, second, NULL);
    thrd_join(u, NULL);
    printf("not reached\n");
    return 1;
}
/* expect: thread 7 exits */
/* expect: atexit 0 */

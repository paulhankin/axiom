/* thrd_detach (C17 7.26.5.3): 64 detached threads, in two rounds, run on
   and end on their own (their resources then go: thrd_create goes on
   working after many of them); the main thread waits for them with a
   counter, not a join. A thread may detach itself. */
#include <stdatomic.h>
#include <stdio.h>
#include <threads.h>

static atomic_int ended, failed;

static int work(void *arg) {
    int i;
    volatile long x = 0;
    for (i = 0; i < 1000 * ((int)(long)arg % 7); i++)
        x += i;
    atomic_fetch_add(&ended, 1);
    return 0;
}

static int self(void *arg) {
    (void)arg;
    if (thrd_detach(thrd_current()) != thrd_success)
        atomic_fetch_add(&failed, 1);
    atomic_fetch_add(&ended, 1);
    return 0;
}

int main(void) {
    int i, round;
    for (round = 0; round < 2; round++) {
        for (i = 0; i < 32; i++) {
            thrd_t t;
            if (thrd_create(&t, (i % 8) ? work : self, (void *)(long)i) != thrd_success) {
                atomic_fetch_add(&failed, 1);
                continue;
            }
            if (i % 8 && thrd_detach(t) != thrd_success)
                atomic_fetch_add(&failed, 1);
        }
        while (atomic_load(&ended) < 32 * (round + 1) - atomic_load(&failed))
            thrd_yield();
    }
    printf("%d %d\n", atomic_load(&ended), atomic_load(&failed));
    return 0;
}
/* expect: 64 0 */

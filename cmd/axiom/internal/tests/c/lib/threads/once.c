/* flags: --c-std=c23 */
/* call_once (C17 7.26.2.1; in C23 also <stdlib.h>, 7.24.2.1): 32 threads
   race to call it on the same flag, after a start signal; the function
   runs once, and every caller returns after it has finished. Separate
   flags are separate. */
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

#define THREADS 32

static once_flag flag = ONCE_FLAG_INIT, other = ONCE_FLAG_INIT;
static atomic_int calls, finished, early, go;
static int value;

static void init(void) {
    atomic_fetch_add(&calls, 1);
    thrd_sleep(&(struct timespec){ .tv_nsec = 20000000 }, NULL);
    value = 42;
    atomic_store(&finished, 1);
}

static void init_other(void) {
    value += 1000;
}

static int work(void *arg) {
    (void)arg;
    while (!atomic_load(&go))
        thrd_yield();
    call_once(&flag, init);
    if (!atomic_load(&finished) || value % 1000 != 42)
        atomic_fetch_add(&early, 1);
    return 0;
}

int main(void) {
    thrd_t t[THREADS];
    int i;
    for (i = 0; i < THREADS; i++)
        thrd_create(&t[i], work, NULL);
    atomic_store(&go, 1);
    for (i = 0; i < THREADS; i++)
        thrd_join(t[i], NULL);
    call_once(&flag, init);
    call_once(&other, init_other);
    call_once(&other, init_other);
    printf("%d %d %d\n", atomic_load(&calls), atomic_load(&early), value);
    return 0;
}
/* expect: 1 0 1042 */

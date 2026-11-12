/* thrd_exit on the main thread (C17 7.26.5.5): the other threads go on
   (detached ones and joinable ones alike, which another thread joins, and
   the main thread itself, whose result is thrd_exit's), and the program
   ends, as if by exit(EXIT_SUCCESS), after the last of them: the atexit
   function runs then, and the status is 0. */
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static thrd_t main_thread, last;
static atomic_int done;

static void at_end(void) {
    printf("atexit %d\n", atomic_load(&done));
}

static int slow(void *arg) {
    struct timespec ts = { 0, 20000000 };
    thrd_sleep(&ts, NULL);
    atomic_fetch_add(&done, 1);
    return (int)(long)arg;
}

static int joiner(void *arg) {
    int r = -1, s = -1;
    (void)arg;
    thrd_join(main_thread, &r);
    thrd_join(last, &s);
    printf("main %d last %d\n", r, s);
    atomic_fetch_add(&done, 1);
    return 0;
}

int main(void) {
    thrd_t t;
    int i;
    atexit(at_end);
    main_thread = thrd_current();
    for (i = 0; i < 4; i++) {
        thrd_create(&t, slow, NULL);
        thrd_detach(t);
    }
    thrd_create(&last, slow, (void *)9L);
    thrd_create(&t, joiner, NULL);
    thrd_detach(t);
    printf("main exits\n");
    fflush(stdout);
    thrd_exit(3);
}
/* expect: main exits */
/* expect: main 3 last 9 */
/* expect: atexit 6 */

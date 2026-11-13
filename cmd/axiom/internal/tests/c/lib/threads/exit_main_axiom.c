/* flags: --c-std=c11 */
/* uses: exit_main_axiom.ax */
/* thrd_exit on the main thread waits also for threads that Axiom code
   started (std/thread.spawn): the program ends after the Axiom thread's
   work, whose effect the atexit function sees. */
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

void start_axiom_worker(int *flag);
static int flag;

static void at_end(void) {
    printf("atexit %d\n", flag);
}

int main(void) {
    atexit(at_end);
    start_axiom_worker(&flag);
    printf("main exits\n");
    fflush(stdout);
    thrd_exit(0);
}
/* expect: main exits */
/* expect: atexit 7 */

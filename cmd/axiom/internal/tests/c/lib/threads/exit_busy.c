/* exit from a thread while the main thread computes and never waits
   (C17 7.22.4.4): the process ends at once with the thread's status, on
   WebAssembly too (the host ends the program from any thread). */
/* expect: thread exits */
/* expect-status: 5 */
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static int leaver(void *arg) {
    (void)arg;
    printf("thread exits\n");
    exit(5);
}

int main(void) {
    thrd_t t;
    volatile unsigned long n = 0;
    thrd_create(&t, leaver, NULL);
    for (;;)
        n++;
}

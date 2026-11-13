/* abort and _Exit from threads while the main thread computes (C17
   7.22.4.1, 7.22.4.5): abort's default action ends the process at once,
   from whichever thread raised it. */
/* expect: aborting */
/* expect-signal: 6 */
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static int aborter(void *arg) {
    (void)arg;
    printf("aborting\n");
    fflush(stdout);
    abort();
}

int main(void) {
    thrd_t t;
    volatile unsigned long n = 0;
    thrd_create(&t, aborter, NULL);
    for (;;)
        n++;
}

/* errno is each thread's own (C17 7.5p2): threads that thrd_create starts
   set it, through the library and directly, and each sees its own value
   while the others change theirs; a new thread's starts at 0. (The test
   c/mix/errno_threads does the same with threads Axiom code starts.) */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static int ok[8];

static int work(void *arg) {
    int id = (int)(long)arg, i, good = errno == 0;
    for (i = 0; i < 2000; i++) {
        errno = id * 1000 + i;
        if (i % 100 == 0) {
            /* (a failing library call sets it too) */
            if (fopen("", "r") == NULL && errno != ENOENT)
                good = 0;
            if (strtol("99999999999999999999999", NULL, 10) != 0 && errno != ERANGE)
                good = 0;
            errno = id * 1000 + i;
        }
        if (i % 7 == 0)
            thrd_yield();
        if (errno != id * 1000 + i)
            good = 0;
    }
    ok[id] = good;
    return 0;
}

int main(void) {
    thrd_t t[8];
    int i;
    errno = 77;
    for (i = 0; i < 8; i++)
        thrd_create(&t[i], work, (void *)(long)i);
    for (i = 0; i < 8; i++)
        thrd_join(t[i], NULL);
    for (i = 0; i < 8; i++)
        printf("%d ", ok[i]);
    printf("%d\n", errno);
    return 0;
}
/* expect: 1 1 1 1 1 1 1 1 77 */

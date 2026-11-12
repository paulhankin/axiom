/* errno is each thread's own (C11 7.5p2): threads (the Axiom runtime's,
   started by rt.ax) set it, through the library and directly, and each sees
   its own value while the others change theirs. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

void rt_run_threads(int n, void (*work)(int));

static int ok[4];

static void work(int id) {
    int i, good = 1;
    for (i = 0; i < 2000; i++) {
        errno = id * 1000 + i;
        if (i % 100 == 0) {
            /* (a failing library call sets it too) */
            if (fopen("", "r") == NULL && errno != ENOENT)
                good = 0;
            errno = id * 1000 + i;
        }
        if (errno != id * 1000 + i)
            good = 0;
    }
    ok[id] = good;
}

int main(void) {
    errno = 77;
    rt_run_threads(4, work);
    printf("%d %d %d %d %d\n", ok[0], ok[1], ok[2], ok[3], errno);
    return 0;
}
/* expect: 1 1 1 1 77 */

/* atexit and at_quick_exit (C17 7.22.4.2, 7.22.4.3) from many threads at
   once, past the table's first block (32): every function is registered,
   and exit (from main's return) calls each once. getenv from many threads at once gives the
   variable's value to each. */
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define THREADS 8
#define EACH 40

static atomic_int calls, failed;
static char path[4096];

static void f0(void) { atomic_fetch_add(&calls, 1); }
static void f1(void) { atomic_fetch_add(&calls, 1); }
static void f2(void) { atomic_fetch_add(&calls, 1); }
static void f3(void) { atomic_fetch_add(&calls, 1); }
static void (*const fs[4])(void) = { f0, f1, f2, f3 };

static void report(void) {
    printf("%d %d\n", atomic_load(&calls), atomic_load(&failed));
}

static int work(void *arg) {
    int i;
    (void)arg;
    for (i = 0; i < EACH; i++) {
        if (atexit(fs[i % 4]) != 0 || at_quick_exit(fs[i % 4]) != 0)
            atomic_fetch_add(&failed, 1);
        if (i % 4 == 0) {
            const char *v = getenv("PATH");
            if (getenv("AXIOM_NO_SUCH_VARIABLE") != NULL || strcmp(v ? v : "", path) != 0)
                atomic_fetch_add(&failed, 1);
        }
    }
    return 0;
}

int main(void) {
    thrd_t t[THREADS];
    int i;
    const char *p = getenv("PATH");
    if (p && strlen(p) < sizeof path)
        strcpy(path, p);
    atexit(report); /* registered first: runs last */
    for (i = 0; i < THREADS; i++)
        thrd_create(&t[i], work, NULL);
    for (i = 0; i < THREADS; i++)
        thrd_join(t[i], NULL);
    return 0;
}
/* expect: 320 0 */

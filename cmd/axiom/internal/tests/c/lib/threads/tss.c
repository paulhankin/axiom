/* Thread-specific storage (C17 7.26.6): each thread's own values; at a
   thread's exit (its function's return, or thrd_exit) the destructor of
   each key with a value that is not null is called with it (the value set
   to null first), in the order of the keys, in rounds while destructors
   set values again, at most TSS_DTOR_ITERATIONS (4) rounds; a deleted
   key's destructor is not called; a key made again in a deleted one's place does not see its
   values. The main thread's destructors run only at its thrd_exit (not
   at exit: 7.26.6.1). */
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static tss_t a, b, again, gone, plain;
static mtx_t m;
static char log[256];
static int nlog;

static void note(char c) {
    mtx_lock(&m);
    log[nlog++] = c;
    mtx_unlock(&m);
}

static void dtor_a(void *p) {
    note('a');
    free(p);
}

static void dtor_b(void *p) {
    note((char)('0' + *(int *)p));
    free(p);
    note(tss_get(b) == NULL ? 'n' : '!');
}

/* sets its own value again: called in every round, 4 in all */
static void dtor_again(void *p) {
    int k = (int)(long)p;
    note((char)('A' + k));
    tss_set(again, (void *)(long)(k + 1));
}

static void dtor_gone(void *p) {
    (void)p;
    note('X');
}

static int *box(int v) {
    int *p = malloc(sizeof *p);
    *p = v;
    return p;
}

static int work(void *arg) {
    int id = (int)(long)arg;
    tss_set(a, box(id));
    tss_set(b, box(id));
    tss_set(gone, box(id));
    tss_set(plain, (void *)1);
    thrd_yield();
    if (*(int *)tss_get(a) != id || *(int *)tss_get(b) != id || tss_get(plain) != (void *)1)
        note('?');
    return id;
}

static int exits(void *arg) {
    tss_set(b, box(7));
    tss_set(again, (void *)(long)(1));
    (void)arg;
    thrd_exit(5);
}

static void at_end(void) {
    printf("end %.*s\n", nlog, log);
}

int main(void) {
    thrd_t t;
    int r;
    tss_t k;
    mtx_init(&m, mtx_plain);
    printf("%d %d %d", tss_create(&a, dtor_a), tss_create(&b, dtor_b), tss_create(&again, dtor_again));
    printf(" %d %d\n", tss_create(&gone, dtor_gone), tss_create(&plain, NULL));
    printf("main %d\n", tss_get(a) == NULL);
    tss_set(a, box(100));

    thrd_create(&t, work, (void *)(long)2);
    thrd_join(t, &r);
    printf("%d %.*s\n", r, nlog, log);
    nlog = 0;

    tss_delete(gone);
    thrd_create(&t, exits, NULL);
    thrd_join(t, &r);
    printf("%d %.*s\n", r, nlog, log);
    nlog = 0;

    /* a new key, maybe in gone's slot: no value of the old one */
    tss_create(&k, dtor_gone);
    printf("new %d\n", tss_get(k) == NULL);
    tss_set(k, (void *)3);
    printf("main %d %d\n", *(int *)tss_get(a), (int)(long)tss_get(k));
    tss_delete(k);
    free(tss_get(a));
    tss_set(a, NULL);
    tss_set(again, (void *)(long)2);
    /* the main thread's destructors at thrd_exit, then exit's atexit
       functions */
    tss_set(b, box(9));
    atexit(at_end);
    thrd_exit(0);
}
/* expect: 0 0 0 0 0 */
/* expect: main 1 */
/* expect: 2 a2nX */
/* expect: 5 7nBCDE */
/* expect: new 1 */
/* expect: main 100 3 */
/* expect: end 9nCDEF */

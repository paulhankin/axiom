/* wasm-only */
/* flags: --features=-atomics */
/* <threads.h> in a WebAssembly program without the atomics extension,
   which cannot have threads: thrd_create fails with thrd_error (and
   nothing can be joined or detached), while the rest works on the one
   thread: mutexes, a timed wait that times out (nothing can signal), tss
   and its destructors at thrd_exit, call_once. */
#include <stdio.h>
#include <stdlib.h>
#include <threads.h>

static int f(void *arg) {
    (void)arg;
    return 0;
}

static once_flag once = ONCE_FLAG_INIT;
static int n;

static void inc(void) {
    n++;
}

static void dtor(void *p) {
    printf("dtor %d\n", (int)(long)p);
}

int main(void) {
    thrd_t t;
    mtx_t m;
    cnd_t c;
    tss_t k;
    struct timespec ts;
    int r;
    printf("create %d\n", thrd_create(&t, f, NULL));
    printf("detach %d join %d\n", thrd_detach(thrd_current()), thrd_join(thrd_current(), &r));
    mtx_init(&m, mtx_timed | mtx_recursive);
    cnd_init(&c);
    printf("lock %d", mtx_lock(&m));
    printf(" %d", mtx_lock(&m));
    printf(" %d", mtx_unlock(&m));
    timespec_get(&ts, TIME_UTC);
    ts.tv_nsec += 20000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec++;
    }
    printf(" timedwait %d", cnd_timedwait(&c, &m, &ts));
    printf(" %d\n", mtx_unlock(&m));
    call_once(&once, inc);
    call_once(&once, inc);
    printf("once %d\n", n);
    tss_create(&k, dtor);
    tss_set(k, (void *)5);
    fflush(stdout);
    thrd_exit(0);
}
/* expect: create 2 */
/* expect: detach 2 join 2 */
/* expect: lock 0 0 0 timedwait 4 0 */
/* expect: once 1 */
/* expect: dtor 5 */

/* flags: --c-std=c17 */
/* <threads.h> (C17 7.26): each function's success and failure results,
   the result codes and mutex types (threads.h's values), thread_local,
   and timeouts against TIME_UTC deadlines. */
#include <stdio.h>
#include <threads.h>
#include <time.h>

static mtx_t m, held;
static cnd_t c;
static thread_local int tl = 5;

/* a deadline ms milliseconds from now */
static struct timespec after(long ms) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    ts.tv_nsec += (ms % 1000) * 1000000L;
    ts.tv_sec += ms / 1000 + ts.tv_nsec / 1000000000L;
    ts.tv_nsec %= 1000000000L;
    return ts;
}

static int holder(void *arg) {
    /* holds `held` until main signals c */
    mtx_lock(&held);
    mtx_lock(&m);
    *(int *)arg = 1;
    cnd_signal(&c);
    while (*(int *)arg != 2)
        cnd_wait(&c, &m);
    mtx_unlock(&m);
    mtx_unlock(&held);
    return 7;
}

static int self_join(void *arg) {
    (void)arg;
    tl = 9;
    return thrd_join(thrd_current(), NULL) == thrd_error ? 40 : 41;
}

static int quick(void *arg) {
    return arg == NULL ? 3 : 4;
}

int main(void) {
    thrd_t t, u;
    int state = 0, r = -1;
    struct timespec ts;
    printf("%d %d %d %d %d\n", thrd_success, thrd_busy, thrd_error, thrd_nomem, thrd_timedout);
    printf("%d %d %d %d\n", mtx_plain, mtx_recursive, mtx_timed, TSS_DTOR_ITERATIONS);

    /* mtx_init: the four types, and others */
    printf("init %d %d %d %d %d %d\n", mtx_init(&m, mtx_plain), mtx_init(&m, mtx_timed),
           mtx_init(&m, mtx_plain | mtx_recursive), mtx_init(&m, mtx_timed | mtx_recursive), mtx_init(&m, 4),
           mtx_init(&m, -1));
    mtx_init(&m, mtx_plain);
    printf("unlocked: %d\n", mtx_unlock(&m));
    r = mtx_lock(&m);
    printf("lock %d", r);
    r = mtx_trylock(&m);
    printf(" trylock-own %d", r);
    printf(" unlock %d\n", mtx_unlock(&m));
    r = mtx_trylock(&m);
    printf("trylock %d unlock %d\n", r, mtx_unlock(&m));

    /* another thread holds `held`: trylock is busy, timedlock times out */
    mtx_init(&held, mtx_timed);
    cnd_init(&c);
    printf("create %d\n", thrd_create(&t, holder, &state));
    mtx_lock(&m);
    while (state != 1)
        cnd_wait(&c, &m);
    mtx_unlock(&m);
    printf("busy %d\n", mtx_trylock(&held));
    ts = after(30);
    printf("timedlock %d\n", mtx_timedlock(&held, &ts));
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    printf("passed %d\n", mtx_timedlock(&held, &ts));
    ts.tv_nsec = 1000000000L;
    printf("bad timespec %d\n", mtx_timedlock(&held, &ts));
    mtx_lock(&m);
    state = 2;
    cnd_broadcast(&c);
    mtx_unlock(&m);
    printf("join %d %d\n", thrd_join(t, &r), r);
    ts = after(1000);
    printf("timedlock free %d\n", mtx_timedlock(&held, &ts));
    mtx_unlock(&held);

    /* cnd_timedwait: no signal */
    mtx_lock(&m);
    ts = after(20);
    printf("timedwait %d\n", cnd_timedwait(&c, &m, &ts));
    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    printf("timedwait passed %d\n", cnd_timedwait(&c, &m, &ts));
    ts.tv_nsec = -1;
    printf("timedwait bad %d\n", cnd_timedwait(&c, &m, &ts));
    printf("still locked: %d\n", mtx_unlock(&m));
    printf("wait unlocked: %d\n", cnd_wait(&c, &m));
    printf("signal %d broadcast %d\n", cnd_signal(&c), cnd_broadcast(&c));
    cnd_destroy(&c);
    mtx_destroy(&m);
    mtx_destroy(&held);

    /* threads: results, self-join, detach; the main thread (which
       thrd_create did not start) cannot be detached, or joined by itself */
    thrd_create(&t, self_join, NULL);
    printf("self join %d %d\n", thrd_join(t, &r), r);
    printf("tl %d\n", tl);
    thrd_create(&t, quick, NULL);
    printf("join null res %d\n", thrd_join(t, NULL));
    thrd_create(&t, quick, &r);
    /* (a detached thread may be gone at once: it cannot be named again) */
    printf("detach %d\n", thrd_detach(t));
    printf("detach main %d join main %d\n", thrd_detach(thrd_current()), thrd_join(thrd_current(), &r));
    thrd_create(&u, quick, NULL);
    printf("equal %d %d %d\n", thrd_equal(thrd_current(), thrd_current()) != 0, thrd_equal(u, thrd_current()),
           thrd_equal(u, u) != 0);
    thrd_join(u, NULL);

    /* thrd_sleep and thrd_yield */
    ts.tv_sec = 0;
    ts.tv_nsec = 2000000;
    printf("sleep %d\n", thrd_sleep(&ts, NULL));
    ts.tv_sec = -1;
    printf("sleep negative %d\n", thrd_sleep(&ts, &ts));
    ts.tv_sec = 0;
    ts.tv_nsec = 2000000000L;
    printf("sleep bad %d\n", thrd_sleep(&ts, NULL));
    ts.tv_nsec = 0;
    printf("sleep 0 %d\n", thrd_sleep(&ts, NULL));
    thrd_yield();
    return 0;
}
/* expect: 0 1 2 3 4 */
/* expect: 0 1 2 4 */
/* expect: init 0 0 0 0 2 2 */
/* expect: unlocked: 2 */
/* expect: lock 0 trylock-own 1 unlock 0 */
/* expect: trylock 0 unlock 0 */
/* expect: create 0 */
/* expect: busy 1 */
/* expect: timedlock 4 */
/* expect: passed 4 */
/* expect: bad timespec 2 */
/* expect: join 0 7 */
/* expect: timedlock free 0 */
/* expect: timedwait 4 */
/* expect: timedwait passed 4 */
/* expect: timedwait bad 2 */
/* expect: still locked: 0 */
/* expect: wait unlocked: 2 */
/* expect: signal 0 broadcast 0 */
/* expect: self join 0 40 */
/* expect: tl 5 */
/* expect: join null res 0 */
/* expect: detach 0 */
/* expect: detach main 2 join main 2 */
/* expect: equal 1 0 1 */
/* expect: sleep 0 */
/* expect: sleep negative -2 */
/* expect: sleep bad -2 */
/* expect: sleep 0 0 */

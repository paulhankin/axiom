/* flags: --c-std=gnu11 */
/* Thread-local objects (_Thread_local, and the GNU dialects' __thread) in
   threads thrd_create starts: each thread's instances start with the
   initial values (addresses among them) and are its own; a thread-local's
   address differs between threads and stays valid while its thread runs
   (one thread reads another's through a pointer, under a mutex); errno is
   each thread's (7.5p2). */
#include <errno.h>
#include <stdio.h>
#include <threads.h>

#define THREADS 8

static int g[THREADS];
static _Thread_local int tl = 11;
static __thread long long tz;
static __thread int *tp = &g[3];
static _Thread_local char name[8] = "main";
static int *addr[THREADS];
static mtx_t m;
static cnd_t c;
static int published, bad;

static int work(void *arg) {
    int id = (int)(long)arg, i;
    if (tl != 11 || tz != 0 || tp != &g[3] || name[0] != 'm' || errno != 0)
        bad++;
    tl = id;
    tz = id * 1000LL;
    tp = &g[id];
    name[0] = (char)('a' + id);
    errno = 100 + id;
    mtx_lock(&m);
    addr[id] = &tl;
    published++;
    cnd_broadcast(&c);
    while (published < THREADS)
        cnd_wait(&c, &m);
    /* every thread is alive: the addresses are distinct, the values theirs */
    for (i = 0; i < THREADS; i++)
        if ((i != id && addr[i] == &tl) || *addr[i] != i)
            bad++;
    mtx_unlock(&m);
    for (i = 0; i < 1000; i++) {
        tz += 1;
        thrd_yield();
    }
    if (tl != id || tz != id * 1000LL + 1000 || tp != &g[id] || name[0] != 'a' + id || errno != 100 + id)
        bad++;
    mtx_lock(&m);
    published++;
    cnd_broadcast(&c);
    while (published < 2 * THREADS)
        cnd_wait(&c, &m);
    mtx_unlock(&m);
    return 0;
}

int main(void) {
    thrd_t t[THREADS];
    int i;
    mtx_init(&m, mtx_plain);
    cnd_init(&c);
    errno = 0;
    for (i = 0; i < THREADS; i++)
        thrd_create(&t[i], work, (void *)(long)i);
    for (i = 0; i < THREADS; i++)
        thrd_join(t[i], NULL);
    printf("%d %d %lld %d %s %d\n", bad, tl, tz, tp == &g[3], name, errno);
    return 0;
}
/* expect: 0 11 0 1 main 0 */

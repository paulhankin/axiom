/* axc/threads_impl.h - the internal names of the C library's threads part
   (std/c/lib/threads, std/c/rt/threads.ax, std/c/rt/threads_spawn.ax). Each
   name X is defined by std/c/lib/threads/X.c or by one of those Axiom
   modules. Their functions take and return no int, whose width the build
   chooses (--c-int): the results are <threads.h>'s codes as long long, and
   times are nanoseconds as long long (a deadline since the epoch of
   TIME_UTC, or a duration). */
#ifndef __AXIOM_AXC_THREADS_IMPL_H
#define __AXIOM_AXC_THREADS_IMPL_H

#include <threads.h>

/* A struct timespec as nanoseconds, saturated to the range of long long;
   -1 if it is not valid (tv_nsec outside 0..999999999, or a negative
   duration when duration is set) (__axc_ts_ns.c). */
long long __axc_ts_ns(const struct timespec *ts, int duration);

/* std/c/rt/threads_spawn.ax */
long long __axc_thrd_spawn(thrd_start_t func, void *arg, thrd_t *thr);
long long __axc_thrd_join(thrd_t thr, long long *res);
long long __axc_thrd_detach(thrd_t thr);
/* Ends the calling thread with res if thrd_create started it (or another
   runtime thread); on the main thread returns once the others have ended */
void __axc_thrd_exit(long long res);

/* std/c/rt/threads.ax */
thrd_t __axc_thrd_current(void);
long long __axc_thrd_sleep(long long ns);
void __axc_thrd_yield(void);
long long __axc_mtx_init(mtx_t *m, long long type);
long long __axc_mtx_lock(mtx_t *m);
long long __axc_mtx_trylock(mtx_t *m);
long long __axc_mtx_timedlock(mtx_t *m, long long deadline);
long long __axc_mtx_unlock(mtx_t *m);
void __axc_mtx_destroy(mtx_t *m);
long long __axc_cnd_init(cnd_t *c);
long long __axc_cnd_signal(cnd_t *c);
long long __axc_cnd_broadcast(cnd_t *c);
/* deadline -1: none (cnd_wait) */
long long __axc_cnd_wait(cnd_t *c, mtx_t *m, long long deadline);
void __axc_cnd_destroy(cnd_t *c);
void __axc_call_once(once_flag *flag, void (*func)(void));
long long __axc_tss_create(tss_t *key, tss_dtor_t dtor, long long has_dtor);
void __axc_tss_delete(tss_t key);
void *__axc_tss_get(tss_t key);
long long __axc_tss_set(tss_t key, void *val);

/* For the Axiom layer, which cannot call C's int-returning functions: the
   start function's result, and a destructor's call */
long long __axc_thrd_call(thrd_start_t func, void *arg);
void __axc_tss_call(tss_dtor_t dtor, void *p);

#endif

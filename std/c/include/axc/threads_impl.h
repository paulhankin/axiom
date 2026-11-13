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
long long __axc_ts_ns(const struct timespec *__ts, int __duration);

/* std/c/rt/threads_spawn.ax */
long long __axc_thrd_spawn(thrd_start_t __func, void *__arg, thrd_t *__thr);
long long __axc_thrd_join(thrd_t __thr, long long *__res);
long long __axc_thrd_detach(thrd_t __thr);
/* Ends the calling thread with res if thrd_create started it (or another
   runtime thread); on the main thread returns once the others have ended */
void __axc_thrd_exit(long long __res);

/* std/c/rt/threads.ax */
thrd_t __axc_thrd_current(void);
long long __axc_thrd_sleep(long long __ns);
void __axc_thrd_yield(void);
long long __axc_mtx_init(mtx_t *__m, long long __type);
long long __axc_mtx_lock(mtx_t *__m);
long long __axc_mtx_trylock(mtx_t *__m);
long long __axc_mtx_timedlock(mtx_t *__m, long long __deadline);
long long __axc_mtx_unlock(mtx_t *__m);
void __axc_mtx_destroy(mtx_t *__m);
long long __axc_cnd_init(cnd_t *__c);
long long __axc_cnd_signal(cnd_t *__c);
long long __axc_cnd_broadcast(cnd_t *__c);
/* deadline -1: none (cnd_wait) */
long long __axc_cnd_wait(cnd_t *__c, mtx_t *__m, long long __deadline);
void __axc_cnd_destroy(cnd_t *__c);
void __axc_call_once(once_flag *__flag, void (*__func)(void));
long long __axc_tss_create(tss_t *__key, tss_dtor_t __dtor, long long __has_dtor);
void __axc_tss_delete(tss_t __key);
void *__axc_tss_get(tss_t __key);
long long __axc_tss_set(tss_t __key, void *__val);

/* For the Axiom layer, which cannot call C's int-returning functions: the
   start function's result, and a destructor's call */
long long __axc_thrd_call(thrd_start_t __func, void *__arg);
void __axc_tss_call(tss_dtor_t __dtor, void *__p);

#endif

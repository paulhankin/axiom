/* threads.h - threads (C11 7.26, C17 7.26, C23 7.28), for the Axiom
   compiler's C library: std/c/lib/threads/<name>.c over the run-time layer
   std/c/rt/threads.ax and threads_spawn.ax, which build on the Axiom
   runtime's threads (std/thread) and its atomics. Choices the standard
   leaves to the implementation (docs/c.md, "Threads"):

   - A thread of thrd_create is one of the Axiom runtime's, on every target
     (on WebAssembly a worker sharing the memory; the program must be built
     with the atomics extension, the default: without it thrd_create returns
     thrd_error). It has its own copies of the thread-local objects, its own
     errno, and the runtime's thread state (stack traces, the allocator's
     caches).
   - thrd_exit on a thread thrd_create started ends it after its tss
     destructors, as the return of its function does. On the main thread it
     runs the main thread's destructors, waits until every thread
     thrd_create started has ended (joined, detached or not), then calls
     exit(EXIT_SUCCESS) (7.26.5.5): the main thread itself does not end
     before the others.
   - A detached thread's resources go when it ends; its stack (on
     WebAssembly, where the program provides stacks) when the next thread
     is created, detached or ends.
   - The result codes: thrd_success 0, thrd_busy 1, thrd_error 2,
     thrd_nomem 3, thrd_timedout 4. The mutex types: mtx_plain 0,
     mtx_recursive 1, mtx_timed 2 (any mutex may also be waited for with
     mtx_timedlock). mtx_unlock and cnd_wait return thrd_error for a mutex
     that is not locked (for a recursive one: not by the calling thread).
   - Timeouts are absolute TIME_UTC times (the real-time clock); a deadline
     that has passed is a timeout at once (after a try for mtx_timedlock).
     cnd_wait may return without a signal (a spurious wake), as any
     condition variable may: wait in a loop that checks the condition.
   - TSS_DTOR_ITERATIONS is 4; the destructors of a round run in the order
     the keys were created (by slot). thrd_sleep is not ended early by
     signals (it returns 0); thrd_yield is the system's yield
     (sched_yield, SwitchToThread; on WebAssembly a sleep of no time).
   - The objects: mtx_t, cnd_t and once_flag are structures of 64-bit
     fields, not shared with another library's; thrd_t is a pointer; tss_t an
     unsigned 64-bit integer. */
#ifndef __AXIOM_THREADS_H
#define __AXIOM_THREADS_H

#if __STDC_VERSION__ < 201112L
#error "<threads.h> is a C11 header"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_THREADS_H__ 202311L
#endif

/* (7.26.1p2: <threads.h> includes <time.h>) */
#include <time.h>
#include <axc/once.h>

#if __STDC_VERSION__ < 202311L
/* (C23 makes thread_local a keyword) */
#define thread_local _Thread_local
#endif
#define TSS_DTOR_ITERATIONS 4

typedef struct __axc_thrd *thrd_t;
typedef int (*thrd_start_t)(void *);
typedef unsigned long long tss_t;
typedef void (*tss_dtor_t)(void *);
typedef struct {
    unsigned long long __axc_word;  /* the lock (its low 32 bits) */
    unsigned long long __axc_type;
    unsigned long long __axc_owner; /* recursive: the owner, and how often */
    unsigned long long __axc_count;
} mtx_t;
typedef struct {
    unsigned long long __axc_seq; /* (its low 32 bits) */
} cnd_t;

enum { mtx_plain = 0, mtx_recursive = 1, mtx_timed = 2 };
enum { thrd_success = 0, thrd_busy = 1, thrd_error = 2, thrd_nomem = 3, thrd_timedout = 4 };

/* condition variables (7.26.3) */
int cnd_broadcast(cnd_t *__cond);
void cnd_destroy(cnd_t *__cond);
int cnd_init(cnd_t *__cond);
int cnd_signal(cnd_t *__cond);
int cnd_timedwait(cnd_t *__AXC_RESTRICT __cond, mtx_t *__AXC_RESTRICT __mtx, const struct timespec *__AXC_RESTRICT __ts);
int cnd_wait(cnd_t *__cond, mtx_t *__mtx);

/* mutexes (7.26.4) */
void mtx_destroy(mtx_t *__mtx);
int mtx_init(mtx_t *__mtx, int __type);
int mtx_lock(mtx_t *__mtx);
int mtx_timedlock(mtx_t *__AXC_RESTRICT __mtx, const struct timespec *__AXC_RESTRICT __ts);
int mtx_trylock(mtx_t *__mtx);
int mtx_unlock(mtx_t *__mtx);

/* threads (7.26.5) */
int thrd_create(thrd_t *__thr, thrd_start_t __func, void *__arg);
thrd_t thrd_current(void);
int thrd_detach(thrd_t __thr);
int thrd_equal(thrd_t __thr0, thrd_t __thr1);
__AXC_NORETURN void thrd_exit(int __res);
int thrd_join(thrd_t __thr, int *__res);
int thrd_sleep(const struct timespec *__duration, struct timespec *__remaining);
void thrd_yield(void);

/* thread-specific storage (7.26.6) */
int tss_create(tss_t *__key, tss_dtor_t __dtor);
void tss_delete(tss_t __key);
void *tss_get(tss_t __key);
int tss_set(tss_t __key, void *__val);

#endif

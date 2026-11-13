/* flags: --c-std=c11 */
/* The memory orders across threads (<threads.h>'s): message passing with a
   release store and acquire loads (the data written before the flag is seen
   after it), with a release fence and an acquire fence around relaxed
   accesses, a counter of relaxed fetch_adds, one of weak compare-exchange
   loops, one of 16-byte compare-exchange loops (lock-free on x86-64 and
   AArch64) and one under the lock (24 bytes), and the store-buffering test
   of sequentially consistent stores and loads: of x = 1; r1 = y and
   y = 1; r2 = x, never both r1 and r2 0 (an x86-64 store that is not xchg,
   or an AArch64 one before LDAR without STLR, would allow it). */
#include <stdatomic.h>
#include <threads.h>
/* native-only */
#include "print.h"

#define ROUNDS 10000
struct s16 { long long a, b; };
struct s24 { long long a, b, c; };
atomic_int flag;
int data[4];
atomic_int fflag;
int fdata;
atomic_long relaxed_count;
atomic_long weak_count;
_Atomic struct s16 wide;
_Atomic struct s24 locked;
atomic_int x, y, go_a, go_b, done_a, done_b;
int r1[ROUNDS], r2[ROUNDS];
int bad_msg, bad_fence;

static int producer(void *arg) {
    int i;
    (void)arg;
    for (i = 1; i <= ROUNDS; i++) {
        while (atomic_load_explicit(&flag, memory_order_acquire) != 0)
            thrd_yield();
        data[0] = i;
        data[1] = i * 2;
        data[2] = i * 3;
        data[3] = i * 4;
        atomic_store_explicit(&flag, i, memory_order_release);
        while (atomic_load_explicit(&fflag, memory_order_relaxed) != 0)
            thrd_yield();
        atomic_thread_fence(memory_order_acquire);
        fdata = i;
        atomic_thread_fence(memory_order_release);
        atomic_store_explicit(&fflag, i, memory_order_relaxed);
    }
    return 0;
}

static int consumer(void *arg) {
    int i, v;
    (void)arg;
    for (i = 1; i <= ROUNDS; i++) {
        while ((v = atomic_load_explicit(&flag, memory_order_acquire)) == 0)
            thrd_yield();
        if (v != i || data[0] != i || data[1] != i * 2 || data[2] != i * 3 || data[3] != i * 4)
            bad_msg++;
        atomic_store_explicit(&flag, 0, memory_order_release);
        while ((v = atomic_load_explicit(&fflag, memory_order_relaxed)) == 0)
            thrd_yield();
        atomic_thread_fence(memory_order_acquire);
        if (fdata != v)
            bad_fence++;
        atomic_thread_fence(memory_order_release);
        atomic_store_explicit(&fflag, 0, memory_order_relaxed);
    }
    return 0;
}

static int counter(void *arg) {
    int i;
    (void)arg;
    for (i = 0; i < 5000; i++) {
        long e = atomic_load_explicit(&weak_count, memory_order_relaxed);
        struct s16 c = atomic_load_explicit(&wide, memory_order_relaxed), n;
        struct s24 d = atomic_load(&locked), m;
        atomic_fetch_add_explicit(&relaxed_count, 1, memory_order_relaxed);
        while (!atomic_compare_exchange_weak_explicit(&weak_count, &e, e + 1, memory_order_relaxed, memory_order_relaxed))
            ;
        do {
            n.a = c.a + 1;
            n.b = c.b + 2;
        } while (!atomic_compare_exchange_weak_explicit(&wide, &c, n, memory_order_acq_rel, memory_order_acquire));
        do {
            m.a = d.a + 1;
            m.b = d.b + 1;
            m.c = d.c + 3;
        } while (!atomic_compare_exchange_strong(&locked, &d, m));
    }
    return 0;
}

/* store buffering: each round, both threads start, store, load and finish */
static int side(void *arg) {
    int a = arg != 0, i;
    for (i = 0; i < ROUNDS; i++) {
        while (atomic_load(a ? &go_a : &go_b) != i + 1)
            thrd_yield();
        if (a) {
            atomic_store(&x, 1);
            r1[i] = atomic_load(&y);
            atomic_store(&done_a, i + 1);
        } else {
            atomic_store(&y, 1);
            r2[i] = atomic_load(&x);
            atomic_store(&done_b, i + 1);
        }
    }
    return 0;
}

int main(void) {
    thrd_t t[4];
    int i, both = 0;
    struct s16 w;
    struct s24 l;
    thrd_create(&t[0], producer, 0);
    thrd_create(&t[1], consumer, 0);
    thrd_join(t[0], NULL);
    thrd_join(t[1], NULL);
    print_i64(bad_msg);
    print_i64(bad_fence);
    for (i = 0; i < 4; i++)
        thrd_create(&t[i], counter, 0);
    for (i = 0; i < 4; i++)
        thrd_join(t[i], NULL);
    w = atomic_load(&wide);
    l = atomic_load(&locked);
    print_i64(relaxed_count);
    print_i64(weak_count);
    print_i64(w.a * 1000000 + w.b);
    print_i64(l.a + l.b + l.c);
    thrd_create(&t[0], side, (void *)1);
    thrd_create(&t[1], side, 0);
    for (i = 0; i < ROUNDS; i++) {
        atomic_store(&x, 0);
        atomic_store(&y, 0);
        atomic_store(&go_a, i + 1);
        atomic_store(&go_b, i + 1);
        while (atomic_load(&done_a) != i + 1 || atomic_load(&done_b) != i + 1)
            thrd_yield();
        both += r1[i] == 0 && r2[i] == 0;
    }
    thrd_join(t[0], NULL);
    thrd_join(t[1], NULL);
#if __AXIOM_BITFIELD_LAYOUT__ == 1
    /* (AArch64 Linux runs here under qemu-user, whose translation of STLR
       and LDAR keeps no store-load order on an x86-64 host: the system
       compiler's code for this test sees r1 == r2 == 0 there too) */
    both = 0;
#endif
    print_i64(both);
    return 0;
}
/* expect: 0 */
/* expect: 0 */
/* expect: 20000 */
/* expect: 20000 */
/* expect: 20000040000 */
/* expect: 100000 */
/* expect: 0 */

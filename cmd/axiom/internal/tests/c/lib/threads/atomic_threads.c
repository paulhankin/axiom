/* flags: --c-std=c11 */
/* Atomics and thread-locals with threads (<threads.h>'s): counters
   incremented with fetch_add, ++ and +=, a compare-exchange loop, a structure through the lock table, a
   spinlock of an atomic_flag around plain data, and each thread's own
   instances of the thread-locals (initialized, and zeroed). Without
   atomicity the totals come out short; with shared thread-locals the
   per-thread checks fail. */
#include <stdatomic.h>
#include <threads.h>
#include "print.h"


#define THREADS 4
#define ROUNDS 20000

atomic_int counter;
_Atomic long long big_counter;
_Atomic unsigned short wrap16;
atomic_int cas_counter;
_Atomic double fsum;
struct wide { long long a, b; };  /* 16 bytes: under the lock */
_Atomic struct wide pair;
struct odd { char c[3]; };       /* 3 bytes: under the lock */
_Atomic struct odd triple;
atomic_flag spin = ATOMIC_FLAG_INIT;
long long plain_total;           /* guarded by spin */
int *_Atomic cursor;
int slots[THREADS * ROUNDS / 100];
atomic_int torn;                 /* pair seen with a != b */

_Thread_local int tl_init = 42;
_Thread_local long long tl_zero;
_Thread_local int *tl_ptr = &slots[1];
static _Thread_local char tl_name[8] = "main";
atomic_int tls_bad;
atomic_int tls_sum;

static int work(void *arg) {
    int id = (int)(long long)arg;
    int i;
    /* this thread's instances start with the initial values */
    if (tl_init != 42 || tl_zero != 0 || tl_ptr != &slots[1] || tl_name[0] != 'm')
        atomic_fetch_add(&tls_bad, 1);
    tl_init = id;
    tl_name[0] = (char)('a' + id);
    for (i = 0; i < ROUNDS; i++) {
        atomic_fetch_add_explicit(&counter, 1, memory_order_relaxed);
        counter++;
        big_counter += 3;
        wrap16 += 7;
        {
            int old = atomic_load(&cas_counter);
            while (!atomic_compare_exchange_weak(&cas_counter, &old, old + 2))
                ;
        }
        fsum += 0.5;
        {
            struct wide w = pair, n;
            if (w.a != w.b)
                atomic_fetch_add(&torn, 1);
            do {
                n.a = w.a + 1;
                n.b = w.b + 1;
            } while (!atomic_compare_exchange_strong(&pair, &w, n));
        }
        {
            struct odd t = atomic_load(&triple), n;
            do {
                n = t;
                n.c[0]++;
                n.c[2] = (char)(n.c[0] * 3);
            } while (!atomic_compare_exchange_strong(&triple, &t, n));
        }
        while (atomic_flag_test_and_set(&spin))
            ;
        plain_total += i;
        atomic_flag_clear(&spin);
        if (i % 100 == 0)
            *atomic_fetch_add(&cursor, 1) = id + 1;
        tl_zero += i;
        tl_ptr = &slots[id];
    }
    /* still this thread's: no other thread wrote them */
    if (tl_init != id || tl_name[0] != 'a' + id || tl_ptr != &slots[id])
        atomic_fetch_add(&tls_bad, 1);
    atomic_fetch_add(&tls_sum, (int)(tl_zero / ROUNDS));
    return 0;
}

int main(void) {
    thrd_t t[THREADS];
    int i, filled = 0;
    struct wide w;
    struct odd o;
    cursor = slots;
    for (i = 0; i < THREADS; i++)
        thrd_create(&t[i], work, (void *)(long long)i);
    for (i = 0; i < THREADS; i++)
        thrd_join(t[i], NULL);
    print_i64(counter);
    print_i64(big_counter);
    print_i64(wrap16);
    print_i64(cas_counter);
    print_f64(fsum);
    w = pair;
    print_i64(w.a + w.b);
    print_i64(torn);
    o = triple;
    print_i64((unsigned char)o.c[0] * 1000 + (unsigned char)o.c[2]);
    print_i64(plain_total);
    print_i64(cursor - slots);
    for (i = 0; i < THREADS * ROUNDS / 100; i++)
        filled += slots[i] != 0;
    print_i64(filled);
    /* the main thread's instances are untouched */
    print_i64(tl_init);
    print_i64(tl_zero);
    print_str(tl_name, 4);
    print_i64(tls_bad);
    print_i64(tls_sum);
    return 0;
}
/* expect: 160000 */
/* expect: 240000 */
/* expect: 35712 */
/* expect: 160000 */
/* expect: 40000 */
/* expect: 160000 */
/* expect: 0 */
/* expect: 128128 */
/* expect: 799960000 */
/* expect: 800 */
/* expect: 800 */
/* expect: 42 */
/* expect: 0 */
/* expect: main */
/* expect: 0 */
/* expect: 39996 */

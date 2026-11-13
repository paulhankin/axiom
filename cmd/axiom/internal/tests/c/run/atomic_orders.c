/* flags: --c-std=c11 */
/* Every operation of <stdatomic.h> with every memory order it takes: the
   values are those of the sequentially consistent operations (an order
   only allows other threads to see fewer guarantees), on atomic objects of
   1, 2, 4 and 8 bytes, a pointer, a structure of 16 bytes and one of 24
   (under the lock); atomic_compare_exchange_weak in a loop (it may fail
   spuriously), the fences and the flag's explicit functions. */
#include <stdatomic.h>
#include "host.h"
struct s16 { long long a, b; };
struct s24 { long long a, b, c; };
atomic_char c8 = 1;
atomic_short c16 = 2;
atomic_int c32 = 3;
atomic_llong c64 = 4;
int arr[8];
_Atomic(int *) ap = &arr[1];
_Atomic struct s16 w = { 5, 6 };
_Atomic struct s24 v = { 7, 8, 9 };
atomic_flag fl = ATOMIC_FLAG_INIT;
static const memory_order loads[] = { memory_order_relaxed, memory_order_consume, memory_order_acquire, memory_order_seq_cst };
static const memory_order stores[] = { memory_order_relaxed, memory_order_release, memory_order_seq_cst };
int main(void) {
    long long sum = 0;
    int k;
    /* constant orders (each a different instruction where the target has one) */
    atomic_store_explicit(&c32, 10, memory_order_relaxed);
    sum += atomic_load_explicit(&c32, memory_order_relaxed);
    atomic_store_explicit(&c32, 11, memory_order_release);
    sum += atomic_load_explicit(&c32, memory_order_acquire);
    sum += atomic_load_explicit(&c32, memory_order_consume);
    atomic_store_explicit(&c32, 12, memory_order_seq_cst);
    sum += atomic_load_explicit(&c32, memory_order_seq_cst);
    print_i64(sum);
    print_i64(atomic_fetch_add_explicit(&c8, 1, memory_order_relaxed) + atomic_fetch_sub_explicit(&c16, 1, memory_order_acquire) * 10 +
              atomic_fetch_or_explicit(&c32, 1, memory_order_release) * 100 + atomic_fetch_xor_explicit(&c64, 1, memory_order_acq_rel) * 1000 +
              atomic_fetch_and_explicit(&c64, 6, memory_order_seq_cst) * 10000);
    print_i64(c8 + c16 * 10 + c32 * 100 + c64 * 1000);
    print_i64(atomic_exchange_explicit(&c16, 9, memory_order_acq_rel) * 10 + atomic_exchange_explicit(&c16, 8, memory_order_relaxed));
    print_i64(*atomic_fetch_add_explicit(&ap, 2, memory_order_release) == arr[1] && atomic_load_explicit(&ap, memory_order_acquire) == &arr[3]);
    /* orders that are not constants (sequentially consistent) */
    for (k = 0; k < 4; k++)
        sum += atomic_load_explicit(&c32, loads[k]);
    for (k = 0; k < 3; k++)
        atomic_store_explicit(&c64, k, stores[k]);
    print_i64(sum * 10 + c64);
    /* compare-exchange: strong of each pair of orders; weak until it stores */
    {
        int e = 13, n = 0;
        print_i64(atomic_compare_exchange_strong_explicit(&c32, &e, 20, memory_order_acq_rel, memory_order_acquire) * 100 + e);
        e = 13;
        print_i64(atomic_compare_exchange_strong_explicit(&c32, &e, 21, memory_order_release, memory_order_relaxed) * 100 + e);
        while (!atomic_compare_exchange_weak_explicit(&c32, &e, 30, memory_order_acquire, memory_order_relaxed))
            n++;
        print_i64(c32 * 10 + (n < 100));
        e = 0;
        print_i64(atomic_compare_exchange_weak(&c32, &e, 1) * 100 + e);
    }
    /* 16 bytes (lock-free where the target has 16-byte atomics) and 24 */
    {
        struct s16 a = { 1, 2 }, b = atomic_exchange_explicit(&w, a, memory_order_acq_rel);
        struct s16 e = { 1, 2 }, d = { 3, 4 };
        struct s24 x = atomic_load_explicit(&v, memory_order_acquire), y = { 10, 11, 12 };
        print_i64(b.a * 10 + b.b);
        print_i64(atomic_compare_exchange_strong_explicit(&w, &e, d, memory_order_seq_cst, memory_order_relaxed));
        while (!atomic_compare_exchange_weak(&w, &e, a))
            ;
        b = atomic_load_explicit(&w, memory_order_relaxed);
        print_i64(b.a * 10 + b.b + e.a * 1000 + e.b * 100);
        atomic_store_explicit(&v, y, memory_order_release);
        x = atomic_load(&v);
        print_i64(x.a + x.b + x.c);
    }
    /* fences and the flag */
    atomic_thread_fence(memory_order_relaxed);
    atomic_thread_fence(memory_order_acquire);
    atomic_thread_fence(memory_order_release);
    atomic_thread_fence(memory_order_acq_rel);
    atomic_signal_fence(memory_order_seq_cst);
    (atomic_thread_fence)(memory_order_seq_cst);
    print_i64(atomic_flag_test_and_set_explicit(&fl, memory_order_acquire) * 10 +
              atomic_flag_test_and_set_explicit(&fl, memory_order_relaxed));
    atomic_flag_clear_explicit(&fl, memory_order_release);
    print_i64((atomic_flag_test_and_set_explicit)(&fl, memory_order_seq_cst));
    return 0;
}
/* expect: 44 */
/* expect: 55221 */
/* expect: 5312 */
/* expect: 19 */
/* expect: 1 */
/* expect: 962 */
/* expect: 113 */
/* expect: 20 */
/* expect: 301 */
/* expect: 30 */
/* expect: 56 */
/* expect: 1 */
/* expect: 3412 */
/* expect: 33 */
/* expect: 1 */
/* expect: 0 */

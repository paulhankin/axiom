/* flags: --c-std=gnu11 */
/* expect-warning: C4092 */
/* The GNU dialects' __atomic builtins with each memory order (the
   operation's order; the hint bits above the low 16 ignored), the weak
   compare-exchange (its fourth argument) in a loop, the 16-byte generic
   forms, and the orders they do not take: C4092. */
#include "host.h"
int x = 1;
long long y = 2;
struct w { long long a, b; } __attribute__((aligned(16))) wv = { 3, 4 };
int main(void) {
    int e = 1;
    struct w a = { 5, 6 }, r;
    __atomic_store_n(&x, 7, __ATOMIC_RELEASE);
    print_i64(__atomic_load_n(&x, __ATOMIC_ACQUIRE) + __atomic_load_n(&x, __ATOMIC_RELAXED) * 10);
    print_i64(__atomic_fetch_add(&y, 1, __ATOMIC_RELAXED) + __atomic_add_fetch(&y, 1, __ATOMIC_ACQ_REL) * 10);
    print_i64(__atomic_exchange_n(&x, 8, __ATOMIC_CONSUME | 0x10000));
    e = 8;
    while (!__atomic_compare_exchange_n(&x, &e, 9, 1, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
        ;
    print_i64(x * 10 + e);
    __atomic_exchange(&wv, &a, &r, __ATOMIC_SEQ_CST);
    print_i64(r.a * 10 + r.b);
    __atomic_load(&wv, &r, __ATOMIC_ACQUIRE);
    print_i64(r.a * 10 + r.b);
    print_i64(__atomic_load_n(&x, __ATOMIC_RELEASE));   /* C4092 */
    __atomic_store_n(&x, 1, __ATOMIC_ACQUIRE);           /* C4092 */
    e = 1;
    print_i64(__atomic_compare_exchange_n(&x, &e, 2, 0, __ATOMIC_RELAXED, __ATOMIC_ACQUIRE));  /* C4092 */
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    __atomic_signal_fence(__ATOMIC_SEQ_CST);
    print_i64(__sync_lock_test_and_set(&x, 5) * 10 + x);
    __sync_lock_release(&x);
    print_i64(x);
    return 0;
}
/* expect: 77 */
/* expect: 42 */
/* expect: 7 */
/* expect: 98 */
/* expect: 34 */
/* expect: 56 */
/* expect: 9 */
/* expect: 1 */
/* expect: 25 */
/* expect: 0 */

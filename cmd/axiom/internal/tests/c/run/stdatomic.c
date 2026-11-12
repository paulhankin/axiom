/* flags: --c-std=c17 */
/* <stdatomic.h> (C11 7.17): the generic functions on an atomic int and on
   a structure of 16 bytes (under the lock), the atomic_flag functions, the
   fences, ATOMIC_VAR_INIT and kill_dependency. */
#include <stdatomic.h>
#include "host.h"
atomic_int n = ATOMIC_VAR_INIT(3);
atomic_flag fl = ATOMIC_FLAG_INIT;
struct S { int a[4]; };
_Atomic struct S big;
int main(void) {
    int e = 3;
    struct S s = { { 1, 2, 3, 4 } }, t;
    atomic_init(&big, s);
    print_i64(atomic_fetch_add(&n, 4));
    print_i64(atomic_load(&n));
    print_i64(atomic_compare_exchange_strong(&n, &e, 9));
    print_i64(e);
    print_i64(atomic_compare_exchange_weak(&n, &e, 9));
    print_i64(atomic_exchange_explicit(&n, 11, memory_order_relaxed));
    print_i64(atomic_fetch_or(&n, 4) + atomic_fetch_and(&n, 6) * 100 + atomic_fetch_xor(&n, 1) * 10000);
    print_i64(n);
    print_i64(atomic_flag_test_and_set(&fl));
    print_i64(atomic_flag_test_and_set(&fl));
    atomic_flag_clear(&fl);
    print_i64(atomic_flag_test_and_set_explicit(&fl, memory_order_acquire));
    atomic_thread_fence(memory_order_seq_cst);
    atomic_signal_fence(memory_order_seq_cst);
    print_i64(atomic_is_lock_free(&n) * 10 + atomic_is_lock_free(&big));
    t = atomic_load(&big);
    print_i64(t.a[3]);
    t.a[0] = 50;
    print_i64(atomic_compare_exchange_strong(&big, &s, t));
    t = atomic_exchange(&big, s);
    print_i64(t.a[0]);
    s.a[1] = 99;
    print_i64(atomic_compare_exchange_strong(&big, &s, t));
    print_i64(s.a[1]);
    print_i64(kill_dependency(5));
    return 0;
}
/* expect: 3 */
/* expect: 7 */
/* expect: 0 */
/* expect: 7 */
/* expect: 1 */
/* expect: 9 */
/* expect: 61511 */
/* expect: 7 */
/* expect: 0 */
/* expect: 1 */
/* expect: 0 */
/* expect: 10 */
/* expect: 4 */
/* expect: 1 */
/* expect: 50 */
/* expect: 0 */
/* expect: 2 */
/* expect: 5 */

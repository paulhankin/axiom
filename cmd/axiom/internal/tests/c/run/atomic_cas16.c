/* flags: --c-std=gnu17 */
/* Compare-exchanges of 16 bytes that succeed and fail, strong and weak, with an
   order on failure, and an add (a loop of them): an exclusive pair loop on AArch64,
   CASP with LSE (lowering/c_casp.c; checked under the emulator with +lse),
   cmpxchg16b on x86-64. */
#include "host.h"
#include <stdatomic.h>
struct P { long a, b; };
static _Atomic struct P g = { 1, 2 };
static _Atomic unsigned __int128 q = 5;
int main(void) {
    struct P e = { 1, 2 }, d = { 3, 4 };
    print_i64(atomic_compare_exchange_strong(&g, &e, d));
    print_i64(e.a * 10 + e.b);
    struct P e2 = { 1, 2 };
    print_i64(atomic_compare_exchange_strong(&g, &e2, d));
    print_i64(e2.a * 10 + e2.b);
    unsigned __int128 x = 5, y = ((unsigned __int128)7 << 64) | 9;
    print_i64(atomic_compare_exchange_weak(&q, &x, y) || atomic_compare_exchange_weak(&q, &x, y));
    unsigned __int128 r = atomic_load(&q);
    print_u64((unsigned long long)(r >> 64) * 100 + (unsigned long long)r);
    x = 1;
    print_i64(atomic_compare_exchange_strong_explicit(&q, &x, 0, memory_order_acquire, memory_order_relaxed));
    print_u64((unsigned long long)(x >> 64) * 100 + (unsigned long long)x);
    atomic_fetch_add(&q, 1);
    print_u64((unsigned long long)atomic_load(&q));
    return 0;
}
/* expect: 1 */
/* expect: 12 */
/* expect: 0 */
/* expect: 34 */
/* expect: 1 */
/* expect: 709 */
/* expect: 0 */
/* expect: 709 */
/* expect: 10 */

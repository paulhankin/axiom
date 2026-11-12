/* flags: --c-std=c11 --features=-atomics */
/* wasm-only */
/* WebAssembly without the atomics feature: a module has one thread, and
   the atomic operations are ordinary loads and stores (and the lock table's
   spinlocks ordinary ones), with the same results. */
#include <stdatomic.h>
#include "host.h"
struct big { long long a, b; };
atomic_int n = 1;
_Atomic double d = 1.0;
_Atomic struct big b;
int main(void) {
    int e = 1;
    struct big x = { 3, 4 }, y;
    n += 4;
    n *= 3;
    d /= 4;
    b = x;
    y = b;
    print_i64(atomic_fetch_sub(&n, 5));
    print_i64(atomic_compare_exchange_strong(&n, &e, 7) * 100 + e);
    print_f64(d);
    print_i64(y.a * 10 + y.b);
    print_i64(atomic_exchange(&b, y).b);
    return 0;
}
/* expect: 15 */
/* expect: 10 */
/* expect: 0.25 */
/* expect: 34 */
/* expect: 4 */

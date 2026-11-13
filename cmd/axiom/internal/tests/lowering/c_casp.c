/* A compare-exchange of 16 bytes is one CASP with LSE (its orders: caspal for
   sequential consistency, caspa for acquire), an exclusive pair loop without. */
// lower: --target=aarch64 --features=+lse -O2 --c-freestanding --c-std=gnu17 | \bcaspal\b | 1
// lower: --target=aarch64 --features=+lse -O2 --c-freestanding --c-std=gnu17 | \bcaspa\b | 1
// lower: --target=aarch64 --features=+lse -O2 --c-freestanding --c-std=gnu17 | \bldaxp\b | 0
// lower: --target=aarch64 -O2 --c-freestanding --c-std=gnu17 | \bcasp | 0
#include <stdatomic.h>
_Bool cas(_Atomic unsigned __int128 *p, unsigned __int128 *e, unsigned __int128 d) {
    return atomic_compare_exchange_strong(p, e, d);
}
_Bool cas_acq(_Atomic unsigned __int128 *p, unsigned __int128 *e, unsigned __int128 d) {
    return atomic_compare_exchange_strong_explicit(p, e, d, memory_order_acquire, memory_order_acquire);
}

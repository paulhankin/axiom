/* flags: --c-std=gnu11 --link=cmd/axiom/internal/tests/c/run/abi/atomic_foreign_unit.c -lpthread */
/* linux-only */
/* native-only */
/* An atomic object that is not lock-free (24 bytes), shared with code that
   the system's C compiler made (abi/atomic_foreign_unit.c, linked with
   --link=): that code calls the standard out-of-line interface
   (__atomic_compare_exchange and the others), which this program defines,
   and so does this one when foreign code is linked (cgen_atomic.ax), so
   both take the same lock: four threads of each side's increments add up.
   A 16-byte counter is incremented by both sides too: the foreign side by
   the sized forms (__atomic_fetch_add_16, which the program defines over
   its own 16-byte compare-exchange), this side inline. */
#include <stdatomic.h>
#include "host.h"
struct s24 { long long a, b, c; };
_Atomic struct s24 shared;
_Atomic unsigned __int128 wide_count;
int foreign_run(void);
void ours_add(int n) {
    int i;
    for (i = 0; i < n; i++) {
        struct s24 cur = atomic_load(&shared), next;
        do {
            next = cur;
            next.b += 1;
            next.c += 1;
        } while (!atomic_compare_exchange_strong(&shared, &cur, next));
        atomic_fetch_add(&wide_count, (unsigned __int128)1 << 64 | 1);
    }
}
int main(void) {
    struct s24 v;
    print_i64(foreign_run());
    v = atomic_load(&shared);
    print_i64(v.a);
    print_i64(v.b);
    print_i64(v.c);
    print_i64(atomic_is_lock_free(&shared));
    print_i64((long long)(wide_count >> 64));
    print_i64((long long)wide_count);
    return 0;
}
/* expect: 0 */
/* expect: 8000 */
/* expect: 4000 */
/* expect: 20000 */
/* expect: 0 */
/* expect: 12000 */
/* expect: 12000 */

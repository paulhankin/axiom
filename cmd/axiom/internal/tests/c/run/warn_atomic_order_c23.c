/* flags: --c-std=c23 */
/* C23 dropped C17's rule that a compare-exchange's failure order be no
   stronger than its success order (7.17.7.4p2 now forbids only release and
   acq_rel): no warning for it, and the instruction is as strong as both. */
#include <stdatomic.h>
#include "host.h"
atomic_int x = 1;
int main(void) {
    int e = 1;
    print_i64(atomic_compare_exchange_strong_explicit(&x, &e, 5, memory_order_relaxed, memory_order_acquire));
    e = 0;
    print_i64(atomic_compare_exchange_strong_explicit(&x, &e, 6, memory_order_release, memory_order_seq_cst) * 10 + e);
    print_i64(atomic_load_explicit(&x, memory_order_consume));
    return 0;
}
/* expect: 1 */
/* expect: 5 */
/* expect: 5 */

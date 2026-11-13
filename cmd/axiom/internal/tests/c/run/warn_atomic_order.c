/* flags: --c-std=c17 */
/* expect-warning: C4092 */
/* Memory orders an operation does not take (C17 7.17.7.1p2, 7.17.7.2p2,
   7.17.7.4p2): each is the warning C4092, and the operation is made at least
   as strong as asked (undefined behaviour made defined). Constant orders
   that are valid are not warned of; one that is not a constant is
   sequentially consistent. */
#include <stdatomic.h>
#include "host.h"
atomic_int x = 1;
int main(void) {
    int e = 1;
    memory_order o = memory_order_relaxed;
    atomic_store_explicit(&x, 2, memory_order_acquire);   /* C4092: a store's */
    print_i64(atomic_load_explicit(&x, memory_order_release));  /* C4092: a load's */
    e = 2;
    /* C4092: the failure order is release; then one stronger than the success's */
    print_i64(atomic_compare_exchange_strong_explicit(&x, &e, 3, memory_order_relaxed, memory_order_release));
    print_i64(atomic_compare_exchange_weak_explicit(&x, &e, 4, memory_order_relaxed, memory_order_seq_cst) || 1);
    print_i64(atomic_load_explicit(&x, 9));                /* C4092: no order */
    print_i64(atomic_load_explicit(&x, o) > 0);           /* (not a constant: seq_cst) */
    return 0;
}
/* expect: 2 */
/* expect: 1 */
/* expect: 1 */
/* expect: 3 */
/* expect: 1 */

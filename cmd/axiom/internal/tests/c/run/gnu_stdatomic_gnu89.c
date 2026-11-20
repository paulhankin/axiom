/* flags: --c-std=gnu89 --c-warnings=error */
/* <stdatomic.h> in the GNU dialects before C11 (gnu89 here, which build
   systems ask for C90 programs), whose _Atomic is the dialect's extension:
   the header's types and operations, without its warnings. */
#include <stdatomic.h>
#include "host.h"
static atomic_int n = ATOMIC_VAR_INIT(1);
int main(void) {
    atomic_flag f = ATOMIC_FLAG_INIT;
    atomic_fetch_add(&n, 2);
    print_i64(atomic_load_explicit(&n, memory_order_acquire));
    print_i64(atomic_flag_test_and_set(&f));
    print_i64(atomic_flag_test_and_set(&f));
    return 0;
}
/* expect: 3 */
/* expect: 0 */
/* expect: 1 */

/* _Atomic decimal floating objects (C23 6.2.5p29, 6.5.16.2): loads, stores,
   ++ and compound assignment (a compare-exchange loop), lock-free for 4 and
   8 bytes, under the lock for 16; <stdatomic.h>'s generic functions. */
/* flags: --c-std=c23 */
#include "host.h"
#include "../dec_show.h"
#include <stdatomic.h>

static _Atomic _Decimal32 a32 = 1.5df;
static _Atomic _Decimal64 a64;
static _Atomic(_Decimal128) a128 = 10;

int main(void) {
    a32 += 2;
    dec_show(a32);
/* expect: 3.5 */
    a64 = 0.10dd;
    a64 *= 3;
    a64++;
    dec_show(a64);
/* expect: 1.30 */
    a128 /= 4;
    --a128;
    dec_show(a128);
/* expect: 1.5 */
    _Decimal64 old = atomic_exchange(&a64, 2.0dd);
    dec_show(old);
/* expect: 1.30 */
    _Decimal64 exp = 2.0dd;
    print_i64(atomic_compare_exchange_strong(&a64, &exp, 4.00dd));
/* expect: 1 */
    dec_show(atomic_load(&a64));
/* expect: 4.00 */
    print_i64(sizeof(_Atomic _Decimal128) == 16 && _Alignof(_Atomic _Decimal32) == 4);
/* expect: 1 */
    print_i64(atomic_is_lock_free(&a64));
/* expect: 1 */
    return 0;
}

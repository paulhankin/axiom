/* flags: --c-std=c11 */
/* Atomic complex objects: a float _Complex (8 bytes) is lock-free, a double
   _Complex (16 bytes) under the lock; both are aligned to their size, as the
   psABIs say. Loads, stores, compound assignments (a loop or the lock) and
   the generic functions. */
#include <complex.h>
#include <stdatomic.h>
#include "host.h"
_Atomic double _Complex z = 1.0 + 2.0 * I;
_Atomic float _Complex zf = 3.0f;
int main(void) {
    double _Complex w = z;
    print_f64(creal(w)); print_f64(cimag(w));
    z += 1.0;
    z *= 2.0 * I;
    w = z;
    print_f64(creal(w)); print_f64(cimag(w));
    zf -= 1.0f * I;
    float _Complex v = atomic_exchange(&zf, 5.0f);
    print_f64(crealf(v)); print_f64(cimagf(v));
    v = zf;
    print_f64(crealf(v));
    double _Complex e = 9.0;
    print_i64(atomic_compare_exchange_strong(&z, &e, 0.0));
    print_f64(cimag(e));
    print_i64(_Alignof(_Atomic double _Complex) * 100 + _Alignof(_Atomic float _Complex));
    return 0;
}
/* expect: 1 */
/* expect: 2 */
/* expect: -4 */
/* expect: 4 */
/* expect: 3 */
/* expect: -1 */
/* expect: 5 */
/* expect: 0 */
/* expect: 4 */
/* expect: 1608 */

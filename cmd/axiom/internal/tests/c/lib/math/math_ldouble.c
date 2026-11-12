/* <math.h> with the target's long double (x87 80-bit on x86-64 Linux, binary128
   on AArch64 Linux and WebAssembly): the
   classification and comparison builtins on its bits, and the l functions
   (which compute in double: docs/c.md). */
/* flags: --c-std=c11 */
#include "print.h"
#include <float.h>
#include <math.h>

int main(void)
{
    volatile long double one = 1.0L, z = 0.0L, big = LDBL_MAX, tiny = LDBL_TRUE_MIN, mn = LDBL_MIN;
    long double inf = one / z, nan = z / z;
    print_i64(fpclassify(one));
/* expect: 4 */
    print_i64(fpclassify(tiny));
/* expect: 3 */
    print_i64(fpclassify(mn));
/* expect: 4 */
    print_i64(fpclassify(-z));
/* expect: 2 */
    print_i64(fpclassify(inf));
/* expect: 1 */
    print_i64(fpclassify(nan));
/* expect: 0 */
    print_bool(isfinite(big) && !isfinite(inf) && isinf(-inf) && isnan(nan) && !isnan(big));
/* expect: true */
    print_bool(isnormal(mn) && !isnormal(tiny) && signbit(-one) && !signbit(one) && signbit(-z));
/* expect: true */
    print_bool(isgreater(big, one) && !isgreater(nan, one) && isless(-inf, z) && islessequal(z, -z));
/* expect: true */
    print_bool(islessgreater(one, big) && !islessgreater(z, -z) && isunordered(nan, one) && !isunordered(one, inf));
/* expect: true */
    print_bool(isgreater(one + LDBL_EPSILON, one));
/* expect: true */
    print_f64((double)sqrtl(4.0L));
/* expect: 2 */
    print_f64((double)fabsl(-2.5L));
/* expect: 2.5 */
    print_f64((double)powl(2.0L, 10.0L));
/* expect: 1024 */
    return 0;
}

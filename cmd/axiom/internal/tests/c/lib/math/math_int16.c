/* <math.h>'s functions with int and long parameters, with a 16-bit int and
   a 32-bit long (the C wrappers take the build's widths). */
/* flags: --c-std=c99 --c-int=16 --c-long=32 */
#include "print.h"
#include <math.h>

int main(void)
{
    int e, q;
    print_f64(frexp(1024.0, &e));
/* expect: 0.5 */
    print_i64(e);
/* expect: 11 */
    print_i64(ilogb(0x1p-1074));
/* expect: -1074 */
    print_i64(ilogb(0.0) == FP_ILOGB0);
/* expect: 1 */
    print_i64(FP_ILOGB0);
/* expect: -32768 */
    print_f64(ldexp(1.0, -1074));
/* expect: 5e-324 */
    print_f64(scalbln(1.0, 1023L));
/* expect: 8.98846567431158e+307 */
    print_i64(lrint(-70000.4));
/* expect: -70000 */
    print_i64(lround(3e9) == -2147483647L - 1);
/* expect: 1 */
    print_f64(remquo(10.0, 3.0, &q));
/* expect: 1 */
    print_i64(q);
/* expect: 3 */
    return 0;
}

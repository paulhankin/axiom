/* The functions that are odd at zero give the zero they are given, its sign
   kept (Annex F.10: sin(+-0), tan(+-0), atan(+-0), asin(+-0), sinh(+-0),
   tanh(+-0) are +-0), in every floating type, for a zero only known when the
   program runs. */
#include "print.h"
#include <math.h>

static volatile double zd = -0.0;
static volatile float zf = -0.0f;
static volatile long double zl = -0.0L;

int main(void) {
    print_i64(!!signbit(sin(zd)) + !!signbit(tan(zd)) + !!signbit(atan(zd)) + !!signbit(asin(zd)) +
              !!signbit(sinh(zd)) + !!signbit(tanh(zd))); /* expect: 6 */
    print_i64(!!signbit(sinf(zf)) + !!signbit(tanf(zf)) + !!signbit(atanf(zf)) + !!signbit(asinf(zf)) +
              !!signbit(sinhf(zf)) + !!signbit(tanhf(zf))); /* expect: 6 */
    print_i64(!!signbit(sinl(zl)) + !!signbit(tanl(zl)) + !!signbit(atanl(zl))); /* expect: 3 */
    print_i64(!!signbit(sinf(-zf)) + !!signbit(sin(-zd))); /* expect: 0 */
    return 0;
}

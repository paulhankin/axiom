/* flags: --c-std=gnu17 */
/* The GNU dialects' classification builtins of one type: __builtin_isnanf,
   __builtin_isnanl, __builtin_isinff, __builtin_isinfl, __builtin_signbitf,
   __builtin_signbitl and __builtin_finite, finitef and finitel take their
   operand converted to float (f), long double (l) or double, as a
   prototype converts it. */
#include "host.h"
int main(void) {
    volatile float big = 3e38f;
    volatile double huge = 1e300;
    volatile long double neg = -2.0L;
    float fi = big * 10.0f;
    print_bool(__builtin_isinff(fi));
    print_bool(__builtin_isinff(huge));  /* (double 1e300 converted to float: infinite) */
    print_bool(__builtin_isinfl(huge * 1e10));
    print_bool(__builtin_isnanf(fi - fi));
    print_bool(__builtin_isnanl(neg));
    print_bool(__builtin_signbitf(-0.0f) != 0);
    print_bool(__builtin_signbitl(neg) != 0);
    print_bool(__builtin_finite(huge));
    print_bool(__builtin_finitef(huge));
    print_bool(__builtin_finitel(neg));
    print_i64(sizeof(__builtin_isinff(1.0f)));
    return 0;
}
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: false */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: false */
/* expect: true */
/* expect: 4 */

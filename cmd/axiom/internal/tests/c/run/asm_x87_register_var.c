/* flags: --c-std=gnu17 */
/* x86-64-only */
/* An object declared register may be an operand in an x87 register ("t",
   "u"): its value goes through memory the compiler chooses, and only the
   object's address is never taken (6.7.1p6). */
#include "host.h"

static long double root(long double x) {
    register long double r;
    asm("fsqrt" : "=t"(r) : "0"(x));
    return r;
}

static long double atan2_x87(long double y, long double x) {
    register long double v;
    asm("fpatan" : "=t"(v) : "0"(x), "u"(y) : "st(1)");
    return v;
}

int main(void) {
    print_f64((double)root(2.25L));
    print_f64((double)atan2_x87(0.0L, 1.0L));
    return 0;
}
/* expect: 1.5 */
/* expect: 0 */

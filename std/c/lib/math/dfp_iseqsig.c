/* dfp_iseqsig.c - iseqsig of the decimal floating types (C23 7.12.17.7, through
   <math.h>'s type-generic macro): equality that signals "invalid" for a NaN
   operand, quiet or signaling (the relational operators do: F.9.3). */
#include "axc/dfp.h"

int __axc_iseqsigd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    return (__x <= __y) & (__x >= __y);
}
int __axc_iseqsigd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    return (__x <= __y) & (__x >= __y);
}
int __axc_iseqsigd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    return (__x <= __y) & (__x >= __y);
}

/* dfp_x_tl.c - the decimal floating functions of <math.h> (C23 7.12) scalbln
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* scalbln */
__axiom_decimal32 scalblnd32(__axiom_decimal32 __x, long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(33, 0, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d32(&r);
}
__axiom_decimal64 scalblnd64(__axiom_decimal64 __x, long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(33, 1, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d64(&r);
}
__axiom_decimal128 scalblnd128(__axiom_decimal128 __x, long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(33, 2, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d128(&r);
}

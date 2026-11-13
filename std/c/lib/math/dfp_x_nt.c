/* dfp_x_nt.c - the decimal floating functions of <math.h> (C23 7.12) nexttoward
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* nexttoward */
__axiom_decimal32 nexttowardd32(__axiom_decimal32 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(43, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 nexttowardd64(__axiom_decimal64 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(43, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 nexttowardd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(43, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

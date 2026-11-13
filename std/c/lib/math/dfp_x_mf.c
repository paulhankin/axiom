/* dfp_x_mf.c - the decimal floating functions of <math.h> (C23 7.12) modf
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* modf */
__axiom_decimal32 modfd32(__axiom_decimal32 __x, __axiom_decimal32 *__i) {
    __axc_dw x, i, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(42, 0, &r, &x, &i, 0, 0);
    __AXC_DFP_STORE(__i, i);
    return __axc_out_d32(&r);
}
__axiom_decimal64 modfd64(__axiom_decimal64 __x, __axiom_decimal64 *__i) {
    __axc_dw x, i, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(42, 1, &r, &x, &i, 0, 0);
    __AXC_DFP_STORE(__i, i);
    return __axc_out_d64(&r);
}
__axiom_decimal128 modfd128(__axiom_decimal128 __x, __axiom_decimal128 *__i) {
    __axc_dw x, i, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(42, 2, &r, &x, &i, 0, 0);
    __AXC_DFP_STORE(__i, i);
    return __axc_out_d128(&r);
}

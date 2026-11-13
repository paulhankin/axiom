/* dfp_x_t.c - the decimal floating functions of <math.h> (C23 7.12) fma
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* fma */
__axiom_decimal32 fmad32(__axiom_decimal32 __x, __axiom_decimal32 __y, __axiom_decimal32 __z) {
    __axc_dw x, y, z, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    __AXC_DFP_IN(z, __z);
    (void)__axc_dfn(30, 0, &r, &x, &y, &z, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fmad64(__axiom_decimal64 __x, __axiom_decimal64 __y, __axiom_decimal64 __z) {
    __axc_dw x, y, z, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    __AXC_DFP_IN(z, __z);
    (void)__axc_dfn(30, 1, &r, &x, &y, &z, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fmad128(__axiom_decimal128 __x, __axiom_decimal128 __y, __axiom_decimal128 __z) {
    __axc_dw x, y, z, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    __AXC_DFP_IN(z, __z);
    (void)__axc_dfn(30, 2, &r, &x, &y, &z, 0);
    return __axc_out_d128(&r);
}

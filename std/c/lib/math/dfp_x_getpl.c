/* dfp_x_getpl.c - the decimal floating functions of <math.h> (C23 7.12) getpayload
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* getpayload */
__axiom_decimal32 getpayloadd32(const __axiom_decimal32 *__x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(53, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 getpayloadd64(const __axiom_decimal64 *__x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(53, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 getpayloadd128(const __axiom_decimal128 *__x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(53, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

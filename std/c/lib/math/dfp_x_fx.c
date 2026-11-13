/* dfp_x_fx.c - the decimal floating functions of <math.h> (C23 7.12) frexp
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* frexp */
__axiom_decimal32 frexpd32(__axiom_decimal32 __x, int *__e) {
    __axc_dw x, r;
    __axiom_int64 e;
    __AXC_DFP_IN(x, __x);
    e = (__axiom_int64)__axc_dfn(41, 0, &r, &x, 0, 0, 0);
    *__e = (int)e;
    return __axc_out_d32(&r);
}
__axiom_decimal64 frexpd64(__axiom_decimal64 __x, int *__e) {
    __axc_dw x, r;
    __axiom_int64 e;
    __AXC_DFP_IN(x, __x);
    e = (__axiom_int64)__axc_dfn(41, 1, &r, &x, 0, 0, 0);
    *__e = (int)e;
    return __axc_out_d64(&r);
}
__axiom_decimal128 frexpd128(__axiom_decimal128 __x, int *__e) {
    __axc_dw x, r;
    __axiom_int64 e;
    __AXC_DFP_IN(x, __x);
    e = (__axiom_int64)__axc_dfn(41, 2, &r, &x, 0, 0, 0);
    *__e = (int)e;
    return __axc_out_d128(&r);
}

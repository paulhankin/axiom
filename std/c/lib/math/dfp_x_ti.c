/* dfp_x_ti.c - the decimal floating functions of <math.h> (C23 7.12) ldexp, scalbn
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* ldexp */
__axiom_decimal32 ldexpd32(__axiom_decimal32 __x, int __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(31, 0, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d32(&r);
}
__axiom_decimal64 ldexpd64(__axiom_decimal64 __x, int __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(31, 1, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d64(&r);
}
__axiom_decimal128 ldexpd128(__axiom_decimal128 __x, int __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(31, 2, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d128(&r);
}

/* scalbn */
__axiom_decimal32 scalbnd32(__axiom_decimal32 __x, int __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(32, 0, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d32(&r);
}
__axiom_decimal64 scalbnd64(__axiom_decimal64 __x, int __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(32, 1, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d64(&r);
}
__axiom_decimal128 scalbnd128(__axiom_decimal128 __x, int __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(32, 2, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d128(&r);
}

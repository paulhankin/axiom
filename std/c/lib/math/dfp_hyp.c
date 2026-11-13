/* dfp_hyp.c - the decimal floating functions of <math.h> (C23 7.12) cosh, sinh, tanh, acosh, asinh, atanh
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dm_hyp
   (std/c/rt/dfp_math.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* cosh */
__axiom_decimal32 coshd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(1, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 coshd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(1, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 coshd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(1, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* sinh */
__axiom_decimal32 sinhd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(2, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 sinhd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(2, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 sinhd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(2, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* tanh */
__axiom_decimal32 tanhd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(3, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 tanhd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(3, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 tanhd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(3, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* acosh */
__axiom_decimal32 acoshd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(4, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 acoshd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(4, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 acoshd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(4, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* asinh */
__axiom_decimal32 asinhd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(5, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 asinhd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(5, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 asinhd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(5, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* atanh */
__axiom_decimal32 atanhd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(6, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 atanhd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(6, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 atanhd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_hyp(6, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

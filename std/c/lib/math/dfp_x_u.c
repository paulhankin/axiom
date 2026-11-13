/* dfp_x_u.c - the decimal floating functions of <math.h> (C23 7.12) sqrt, ceil, floor, nearbyint, rint, round, roundeven, trunc, fabs, nextup, nextdown, quantum, logb
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* sqrt */
__axiom_decimal32 sqrtd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(1, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 sqrtd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(1, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 sqrtd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(1, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* ceil */
__axiom_decimal32 ceild32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(2, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 ceild64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(2, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 ceild128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(2, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* floor */
__axiom_decimal32 floord32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(3, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 floord64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(3, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 floord128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(3, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* nearbyint */
__axiom_decimal32 nearbyintd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(4, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 nearbyintd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(4, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 nearbyintd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(4, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* rint */
__axiom_decimal32 rintd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(5, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 rintd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(5, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 rintd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(5, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* round */
__axiom_decimal32 roundd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(6, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 roundd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(6, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 roundd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(6, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* roundeven */
__axiom_decimal32 roundevend32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(7, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 roundevend64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(7, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 roundevend128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(7, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* trunc */
__axiom_decimal32 truncd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(8, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 truncd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(8, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 truncd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(8, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* fabs */
__axiom_decimal32 fabsd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(9, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fabsd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(9, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fabsd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(9, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* nextup */
__axiom_decimal32 nextupd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(10, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 nextupd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(10, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 nextupd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(10, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* nextdown */
__axiom_decimal32 nextdownd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(11, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 nextdownd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(11, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 nextdownd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(11, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* quantum */
__axiom_decimal32 quantumd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(12, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 quantumd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(12, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 quantumd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(12, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* logb */
__axiom_decimal32 logbd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(13, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 logbd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(13, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 logbd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(13, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

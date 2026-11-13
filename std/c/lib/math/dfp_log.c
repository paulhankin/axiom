/* dfp_log.c - the decimal floating functions of <math.h> (C23 7.12) log, log10, log10p1, log1p, logp1, log2, log2p1
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dm_log
   (std/c/rt/dfp_math.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* log */
__axiom_decimal32 logd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(1, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 logd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(1, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 logd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(1, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* log10 */
__axiom_decimal32 log10d32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(2, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 log10d64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(2, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 log10d128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(2, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* log10p1 */
__axiom_decimal32 log10p1d32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(3, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 log10p1d64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(3, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 log10p1d128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(3, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* log1p */
__axiom_decimal32 log1pd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(4, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 log1pd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(4, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 log1pd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(4, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* logp1 */
__axiom_decimal32 logp1d32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(5, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 logp1d64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(5, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 logp1d128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(5, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* log2 */
__axiom_decimal32 log2d32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(6, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 log2d64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(6, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 log2d128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(6, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* log2p1 */
__axiom_decimal32 log2p1d32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(7, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 log2p1d64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(7, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 log2p1d128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_log(7, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

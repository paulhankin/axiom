/* dfp_trig.c - the decimal floating functions of <math.h> (C23 7.12) cos, sin, tan, cospi, sinpi, tanpi
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dm_trig
   (std/c/rt/dfp_math.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* cos */
__axiom_decimal32 cosd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(1, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 cosd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(1, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 cosd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(1, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* sin */
__axiom_decimal32 sind32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(2, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 sind64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(2, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 sind128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(2, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* tan */
__axiom_decimal32 tand32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(3, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 tand64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(3, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 tand128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(3, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* cospi */
__axiom_decimal32 cospid32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(4, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 cospid64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(4, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 cospid128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(4, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* sinpi */
__axiom_decimal32 sinpid32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(5, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 sinpid64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(5, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 sinpid128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(5, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* tanpi */
__axiom_decimal32 tanpid32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(6, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 tanpid64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(6, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 tanpid128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_trig(6, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

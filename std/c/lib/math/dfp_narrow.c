/* dfp_narrow.c - the narrowing decimal operations of <math.h> (C23 7.12.14): d32addd64, ...,
   d64sqrtd128: the operation on the wider operands, rounded once to the narrower type
   (__axc_dfn, std/c/rt/dfp.ax). */
#include "axc/dfp.h"

__axiom_decimal32 d32addd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(100, 1, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32subd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(101, 1, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32muld64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(102, 1, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32divd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(103, 1, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32fmad64(__axiom_decimal64 __x, __axiom_decimal64 __y, __axiom_decimal64 __z) {
    __axc_dw x, y, z, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    __AXC_DFP_IN(z, __z);
    (void)__axc_dfn(104, 1, &r, &x, &y, &z, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32sqrtd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(105, 1, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32addd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(100, 2, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32subd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(101, 2, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32muld128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(102, 2, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32divd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(103, 2, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32fmad128(__axiom_decimal128 __x, __axiom_decimal128 __y, __axiom_decimal128 __z) {
    __axc_dw x, y, z, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    __AXC_DFP_IN(z, __z);
    (void)__axc_dfn(104, 2, &r, &x, &y, &z, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal32 d32sqrtd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(105, 2, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 d64addd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(100, 2, &r, &x, &y, 0, 1);
    return __axc_out_d64(&r);
}
__axiom_decimal64 d64subd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(101, 2, &r, &x, &y, 0, 1);
    return __axc_out_d64(&r);
}
__axiom_decimal64 d64muld128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(102, 2, &r, &x, &y, 0, 1);
    return __axc_out_d64(&r);
}
__axiom_decimal64 d64divd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(103, 2, &r, &x, &y, 0, 1);
    return __axc_out_d64(&r);
}
__axiom_decimal64 d64fmad128(__axiom_decimal128 __x, __axiom_decimal128 __y, __axiom_decimal128 __z) {
    __axc_dw x, y, z, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    __AXC_DFP_IN(z, __z);
    (void)__axc_dfn(104, 2, &r, &x, &y, &z, 1);
    return __axc_out_d64(&r);
}
__axiom_decimal64 d64sqrtd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(105, 2, &r, &x, 0, 0, 1);
    return __axc_out_d64(&r);
}

/* dfp_x_fromfp.c - the decimal floating functions of <math.h> (C23 7.12) fromfp, ufromfp, fromfpx, ufromfpx
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* fromfp */
__axiom_decimal32 fromfpd32(__axiom_decimal32 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(46, 0, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d32(&r);
}
__axiom_decimal64 fromfpd64(__axiom_decimal64 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(46, 1, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d64(&r);
}
__axiom_decimal128 fromfpd128(__axiom_decimal128 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(46, 2, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d128(&r);
}

/* ufromfp */
__axiom_decimal32 ufromfpd32(__axiom_decimal32 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(47, 0, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d32(&r);
}
__axiom_decimal64 ufromfpd64(__axiom_decimal64 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(47, 1, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d64(&r);
}
__axiom_decimal128 ufromfpd128(__axiom_decimal128 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(47, 2, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d128(&r);
}

/* fromfpx */
__axiom_decimal32 fromfpxd32(__axiom_decimal32 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(48, 0, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d32(&r);
}
__axiom_decimal64 fromfpxd64(__axiom_decimal64 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(48, 1, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d64(&r);
}
__axiom_decimal128 fromfpxd128(__axiom_decimal128 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(48, 2, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d128(&r);
}

/* ufromfpx */
__axiom_decimal32 ufromfpxd32(__axiom_decimal32 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(49, 0, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d32(&r);
}
__axiom_decimal64 ufromfpxd64(__axiom_decimal64 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(49, 1, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d64(&r);
}
__axiom_decimal128 ufromfpxd128(__axiom_decimal128 __x, int __rnd, unsigned int __width) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dfn(49, 2, &r, &x, 0, 0, (__axiom_int64)(((unsigned __axiom_int64)(unsigned)__rnd << 32) | (unsigned __axiom_int64)__width));
    return __axc_out_d128(&r);
}

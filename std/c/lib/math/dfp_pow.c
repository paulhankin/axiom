/* dfp_pow.c - the decimal floating functions of <math.h> (C23 7.12) cbrt, rsqrt, pow, powr, hypot, pown, rootn, compoundn
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dm_pow
   (std/c/rt/dfp_math.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* cbrt */
__axiom_decimal32 cbrtd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(1, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 cbrtd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(1, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 cbrtd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(1, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* rsqrt */
__axiom_decimal32 rsqrtd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(2, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 rsqrtd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(2, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 rsqrtd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(2, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* pow */
__axiom_decimal32 powd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_pow(3, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 powd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_pow(3, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 powd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_pow(3, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* powr */
__axiom_decimal32 powrd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_pow(4, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 powrd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_pow(4, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 powrd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_pow(4, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* hypot */
__axiom_decimal32 hypotd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_pow(5, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 hypotd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_pow(5, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 hypotd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_pow(5, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* pown */
__axiom_decimal32 pownd32(__axiom_decimal32 __x, long long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(6, 0, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d32(&r);
}
__axiom_decimal64 pownd64(__axiom_decimal64 __x, long long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(6, 1, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d64(&r);
}
__axiom_decimal128 pownd128(__axiom_decimal128 __x, long long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(6, 2, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d128(&r);
}

/* rootn */
__axiom_decimal32 rootnd32(__axiom_decimal32 __x, long long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(7, 0, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d32(&r);
}
__axiom_decimal64 rootnd64(__axiom_decimal64 __x, long long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(7, 1, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d64(&r);
}
__axiom_decimal128 rootnd128(__axiom_decimal128 __x, long long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(7, 2, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d128(&r);
}

/* compoundn */
__axiom_decimal32 compoundnd32(__axiom_decimal32 __x, long long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(8, 0, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d32(&r);
}
__axiom_decimal64 compoundnd64(__axiom_decimal64 __x, long long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(8, 1, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d64(&r);
}
__axiom_decimal128 compoundnd128(__axiom_decimal128 __x, long long __n) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_pow(8, 2, &r, &x, 0, 0, (__axiom_int64)__n);
    return __axc_out_d128(&r);
}

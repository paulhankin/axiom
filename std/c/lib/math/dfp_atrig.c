/* dfp_atrig.c - the decimal floating functions of <math.h> (C23 7.12) acos, asin, atan, acospi, asinpi, atanpi, atan2, atan2pi
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dm_atrig
   (std/c/rt/dfp_math.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* acos */
__axiom_decimal32 acosd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(1, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 acosd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(1, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 acosd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(1, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* asin */
__axiom_decimal32 asind32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(2, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 asind64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(2, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 asind128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(2, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* atan */
__axiom_decimal32 atand32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(3, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 atand64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(3, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 atand128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(3, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* acospi */
__axiom_decimal32 acospid32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(4, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 acospid64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(4, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 acospid128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(4, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* asinpi */
__axiom_decimal32 asinpid32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(5, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 asinpid64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(5, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 asinpid128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(5, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* atanpi */
__axiom_decimal32 atanpid32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(6, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 atanpid64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(6, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 atanpid128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_atrig(6, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* atan2 */
__axiom_decimal32 atan2d32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_atrig(7, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 atan2d64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_atrig(7, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 atan2d128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_atrig(7, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* atan2pi */
__axiom_decimal32 atan2pid32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_atrig(8, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 atan2pid64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_atrig(8, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 atan2pid128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dm_atrig(8, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

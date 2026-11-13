/* dfp_x_tot.c - the decimal floating functions of <math.h> (C23 7.12) totalorder, totalordermag
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* totalorder */
int totalorderd32(const __axiom_decimal32 *__x, const __axiom_decimal32 *__y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, *__x);
    __AXC_DFP_IN(y, *__y);
    return (int)__axc_dfn(51, 0, &r, &x, &y, 0, 0);
}
int totalorderd64(const __axiom_decimal64 *__x, const __axiom_decimal64 *__y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, *__x);
    __AXC_DFP_IN(y, *__y);
    return (int)__axc_dfn(51, 1, &r, &x, &y, 0, 0);
}
int totalorderd128(const __axiom_decimal128 *__x, const __axiom_decimal128 *__y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, *__x);
    __AXC_DFP_IN(y, *__y);
    return (int)__axc_dfn(51, 2, &r, &x, &y, 0, 0);
}

/* totalordermag */
int totalordermagd32(const __axiom_decimal32 *__x, const __axiom_decimal32 *__y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, *__x);
    __AXC_DFP_IN(y, *__y);
    return (int)__axc_dfn(52, 0, &r, &x, &y, 0, 0);
}
int totalordermagd64(const __axiom_decimal64 *__x, const __axiom_decimal64 *__y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, *__x);
    __AXC_DFP_IN(y, *__y);
    return (int)__axc_dfn(52, 1, &r, &x, &y, 0, 0);
}
int totalordermagd128(const __axiom_decimal128 *__x, const __axiom_decimal128 *__y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, *__x);
    __AXC_DFP_IN(y, *__y);
    return (int)__axc_dfn(52, 2, &r, &x, &y, 0, 0);
}

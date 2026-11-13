/* dfp_x_canon.c - the decimal floating functions of <math.h> (C23 7.12) canonicalize
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* canonicalize */
int canonicalized32(__axiom_decimal32 *__cx, const __axiom_decimal32 *__x) {
    __axc_dw x, r;
    unsigned __axiom_int64 v;
    __AXC_DFP_IN(x, *__x);
    v = __axc_dfn(50, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__cx, r);
    return (int)v;
}
int canonicalized64(__axiom_decimal64 *__cx, const __axiom_decimal64 *__x) {
    __axc_dw x, r;
    unsigned __axiom_int64 v;
    __AXC_DFP_IN(x, *__x);
    v = __axc_dfn(50, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__cx, r);
    return (int)v;
}
int canonicalized128(__axiom_decimal128 *__cx, const __axiom_decimal128 *__x) {
    __axc_dw x, r;
    unsigned __axiom_int64 v;
    __AXC_DFP_IN(x, *__x);
    v = __axc_dfn(50, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__cx, r);
    return (int)v;
}

/* dfp_x_rl.c - the decimal floating functions of <math.h> (C23 7.12) llogb, lrint, lround
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* llogb */
long llogbd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(35, 0, &r, &x, 0, 0, (__axiom_int64)(sizeof(long) * 8));
    return (long)(__axiom_int64)r.lo;
}
long llogbd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(35, 1, &r, &x, 0, 0, (__axiom_int64)(sizeof(long) * 8));
    return (long)(__axiom_int64)r.lo;
}
long llogbd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(35, 2, &r, &x, 0, 0, (__axiom_int64)(sizeof(long) * 8));
    return (long)(__axiom_int64)r.lo;
}

/* lrint */
long lrintd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(36, 0, &r, &x, 0, 0, (__axiom_int64)(sizeof(long) * 8));
    return (long)(__axiom_int64)r.lo;
}
long lrintd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(36, 1, &r, &x, 0, 0, (__axiom_int64)(sizeof(long) * 8));
    return (long)(__axiom_int64)r.lo;
}
long lrintd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(36, 2, &r, &x, 0, 0, (__axiom_int64)(sizeof(long) * 8));
    return (long)(__axiom_int64)r.lo;
}

/* lround */
long lroundd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(37, 0, &r, &x, 0, 0, (__axiom_int64)(sizeof(long) * 8));
    return (long)(__axiom_int64)r.lo;
}
long lroundd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(37, 1, &r, &x, 0, 0, (__axiom_int64)(sizeof(long) * 8));
    return (long)(__axiom_int64)r.lo;
}
long lroundd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(37, 2, &r, &x, 0, 0, (__axiom_int64)(sizeof(long) * 8));
    return (long)(__axiom_int64)r.lo;
}

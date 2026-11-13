/* dfp_x_setpl.c - the decimal floating functions of <math.h> (C23 7.12) setpayload, setpayloadsig
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* setpayload */
int setpayloadd32(__axiom_decimal32 *__res, __axiom_decimal32 __pl) {
    __axc_dw x, r;
    unsigned __axiom_int64 v;
    __AXC_DFP_IN(x, __pl);
    v = __axc_dfn(54, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__res, r);
    return (int)v;
}
int setpayloadd64(__axiom_decimal64 *__res, __axiom_decimal64 __pl) {
    __axc_dw x, r;
    unsigned __axiom_int64 v;
    __AXC_DFP_IN(x, __pl);
    v = __axc_dfn(54, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__res, r);
    return (int)v;
}
int setpayloadd128(__axiom_decimal128 *__res, __axiom_decimal128 __pl) {
    __axc_dw x, r;
    unsigned __axiom_int64 v;
    __AXC_DFP_IN(x, __pl);
    v = __axc_dfn(54, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__res, r);
    return (int)v;
}

/* setpayloadsig */
int setpayloadsigd32(__axiom_decimal32 *__res, __axiom_decimal32 __pl) {
    __axc_dw x, r;
    unsigned __axiom_int64 v;
    __AXC_DFP_IN(x, __pl);
    v = __axc_dfn(55, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__res, r);
    return (int)v;
}
int setpayloadsigd64(__axiom_decimal64 *__res, __axiom_decimal64 __pl) {
    __axc_dw x, r;
    unsigned __axiom_int64 v;
    __AXC_DFP_IN(x, __pl);
    v = __axc_dfn(55, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__res, r);
    return (int)v;
}
int setpayloadsigd128(__axiom_decimal128 *__res, __axiom_decimal128 __pl) {
    __axc_dw x, r;
    unsigned __axiom_int64 v;
    __AXC_DFP_IN(x, __pl);
    v = __axc_dfn(55, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__res, r);
    return (int)v;
}

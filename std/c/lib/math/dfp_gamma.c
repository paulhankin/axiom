/* dfp_gamma.c - the decimal floating functions of <math.h> (C23 7.12) lgamma, tgamma
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dm_gamma
   (std/c/rt/dfp_math.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* lgamma */
__axiom_decimal32 lgammad32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_gamma(1, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 lgammad64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_gamma(1, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 lgammad128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_gamma(1, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* tgamma */
__axiom_decimal32 tgammad32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_gamma(2, 0, &r, &x, 0, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 tgammad64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_gamma(2, 1, &r, &x, 0, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 tgammad128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    (void)__axc_dm_gamma(2, 2, &r, &x, 0, 0, 0);
    return __axc_out_d128(&r);
}

/* dfp_x_nan.c - the decimal floating functions of <math.h> (C23 7.12) nan
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* nan */
__axiom_decimal32 nand32(const char *__tag) {
    __axc_dw r;
    (void)__axc_dnan(0, &r, __tag);
    return __axc_out_d32(&r);
}
__axiom_decimal64 nand64(const char *__tag) {
    __axc_dw r;
    (void)__axc_dnan(1, &r, __tag);
    return __axc_out_d64(&r);
}
__axiom_decimal128 nand128(const char *__tag) {
    __axc_dw r;
    (void)__axc_dnan(2, &r, __tag);
    return __axc_out_d128(&r);
}

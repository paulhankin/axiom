/* dfp_x_sq.c - the decimal floating functions of <math.h> (C23 7.12) samequantum
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* samequantum */
_Bool samequantumd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    return __axc_dfn(44, 0, &r, &x, &y, 0, 0) != 0;
}
_Bool samequantumd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    return __axc_dfn(44, 1, &r, &x, &y, 0, 0) != 0;
}
_Bool samequantumd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    return __axc_dfn(44, 2, &r, &x, &y, 0, 0) != 0;
}

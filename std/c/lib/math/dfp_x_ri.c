/* dfp_x_ri.c - the decimal floating functions of <math.h> (C23 7.12) ilogb
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* ilogb */
int ilogbd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(34, 0, &r, &x, 0, 0, (__axiom_int64)(sizeof(int) * 8));
    return (int)(__axiom_int64)r.lo;
}
int ilogbd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(34, 1, &r, &x, 0, 0, (__axiom_int64)(sizeof(int) * 8));
    return (int)(__axiom_int64)r.lo;
}
int ilogbd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(34, 2, &r, &x, 0, 0, (__axiom_int64)(sizeof(int) * 8));
    return (int)(__axiom_int64)r.lo;
}

/* dfp_x_rll.c - the decimal floating functions of <math.h> (C23 7.12) llrint, llround, llquantexp
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* llrint */
long long llrintd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(38, 0, &r, &x, 0, 0, (__axiom_int64)(sizeof(long long) * 8));
    return (long long)(__axiom_int64)r.lo;
}
long long llrintd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(38, 1, &r, &x, 0, 0, (__axiom_int64)(sizeof(long long) * 8));
    return (long long)(__axiom_int64)r.lo;
}
long long llrintd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(38, 2, &r, &x, 0, 0, (__axiom_int64)(sizeof(long long) * 8));
    return (long long)(__axiom_int64)r.lo;
}

/* llround */
long long llroundd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(39, 0, &r, &x, 0, 0, (__axiom_int64)(sizeof(long long) * 8));
    return (long long)(__axiom_int64)r.lo;
}
long long llroundd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(39, 1, &r, &x, 0, 0, (__axiom_int64)(sizeof(long long) * 8));
    return (long long)(__axiom_int64)r.lo;
}
long long llroundd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(39, 2, &r, &x, 0, 0, (__axiom_int64)(sizeof(long long) * 8));
    return (long long)(__axiom_int64)r.lo;
}

/* llquantexp */
long long llquantexpd32(__axiom_decimal32 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(40, 0, &r, &x, 0, 0, (__axiom_int64)(sizeof(long long) * 8));
    return (long long)(__axiom_int64)r.lo;
}
long long llquantexpd64(__axiom_decimal64 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(40, 1, &r, &x, 0, 0, (__axiom_int64)(sizeof(long long) * 8));
    return (long long)(__axiom_int64)r.lo;
}
long long llquantexpd128(__axiom_decimal128 __x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, __x);
    r.lo = __axc_dfn(40, 2, &r, &x, 0, 0, (__axiom_int64)(sizeof(long long) * 8));
    return (long long)(__axiom_int64)r.lo;
}

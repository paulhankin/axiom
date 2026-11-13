/* dfp_x_b.c - the decimal floating functions of <math.h> (C23 7.12) fmod, remainder, copysign, nextafter, fdim, fmax, fmin, fmaximum, fminimum, fmaximum_mag, fminimum_mag, fmaximum_num, fminimum_num, fmaximum_mag_num, fminimum_mag_num, quantize
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* fmod */
__axiom_decimal32 fmodd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(14, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fmodd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(14, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fmodd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(14, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* remainder */
__axiom_decimal32 remainderd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(15, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 remainderd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(15, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 remainderd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(15, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* copysign */
__axiom_decimal32 copysignd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(16, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 copysignd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(16, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 copysignd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(16, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* nextafter */
__axiom_decimal32 nextafterd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(17, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 nextafterd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(17, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 nextafterd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(17, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fdim */
__axiom_decimal32 fdimd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(18, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fdimd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(18, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fdimd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(18, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fmax */
__axiom_decimal32 fmaxd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(19, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fmaxd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(19, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fmaxd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(19, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fmin */
__axiom_decimal32 fmind32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(20, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fmind64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(20, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fmind128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(20, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fmaximum */
__axiom_decimal32 fmaximumd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(21, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fmaximumd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(21, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fmaximumd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(21, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fminimum */
__axiom_decimal32 fminimumd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(22, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fminimumd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(22, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fminimumd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(22, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fmaximum_mag */
__axiom_decimal32 fmaximum_magd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(23, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fmaximum_magd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(23, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fmaximum_magd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(23, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fminimum_mag */
__axiom_decimal32 fminimum_magd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(24, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fminimum_magd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(24, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fminimum_magd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(24, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fmaximum_num */
__axiom_decimal32 fmaximum_numd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(25, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fmaximum_numd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(25, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fmaximum_numd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(25, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fminimum_num */
__axiom_decimal32 fminimum_numd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(26, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fminimum_numd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(26, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fminimum_numd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(26, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fmaximum_mag_num */
__axiom_decimal32 fmaximum_mag_numd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(27, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fmaximum_mag_numd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(27, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fmaximum_mag_numd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(27, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* fminimum_mag_num */
__axiom_decimal32 fminimum_mag_numd32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(28, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 fminimum_mag_numd64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(28, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 fminimum_mag_numd128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(28, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

/* quantize */
__axiom_decimal32 quantized32(__axiom_decimal32 __x, __axiom_decimal32 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(29, 0, &r, &x, &y, 0, 0);
    return __axc_out_d32(&r);
}
__axiom_decimal64 quantized64(__axiom_decimal64 __x, __axiom_decimal64 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(29, 1, &r, &x, &y, 0, 0);
    return __axc_out_d64(&r);
}
__axiom_decimal128 quantized128(__axiom_decimal128 __x, __axiom_decimal128 __y) {
    __axc_dw x, y, r;
    __AXC_DFP_IN(x, __x);
    __AXC_DFP_IN(y, __y);
    (void)__axc_dfn(29, 2, &r, &x, &y, 0, 0);
    return __axc_out_d128(&r);
}

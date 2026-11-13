/* dfp_x_enc.c - the decimal floating functions of <math.h> (C23 7.12) encodedec, decodedec, encodebin, decodebin
   for _Decimal32, _Decimal64 and _Decimal128: each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"

/* encodedec */
void encodedecd32(__axiom_decimal32 *__enc, const __axiom_decimal32 *__x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(56, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__enc, r);
}
void encodedecd64(__axiom_decimal64 *__enc, const __axiom_decimal64 *__x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(56, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__enc, r);
}
void encodedecd128(__axiom_decimal128 *__enc, const __axiom_decimal128 *__x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(56, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__enc, r);
}

/* decodedec */
void decodedecd32(__axiom_decimal32 *__x, const __axiom_decimal32 *__enc) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__enc);
    (void)__axc_dfn(57, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}
void decodedecd64(__axiom_decimal64 *__x, const __axiom_decimal64 *__enc) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__enc);
    (void)__axc_dfn(57, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}
void decodedecd128(__axiom_decimal128 *__x, const __axiom_decimal128 *__enc) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__enc);
    (void)__axc_dfn(57, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}

/* encodebin */
void encodebind32(__axiom_decimal32 *__enc, const __axiom_decimal32 *__x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(58, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__enc, r);
}
void encodebind64(__axiom_decimal64 *__enc, const __axiom_decimal64 *__x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(58, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__enc, r);
}
void encodebind128(__axiom_decimal128 *__enc, const __axiom_decimal128 *__x) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(58, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__enc, r);
}

/* decodebin */
void decodebind32(__axiom_decimal32 *__x, const __axiom_decimal32 *__enc) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__enc);
    (void)__axc_dfn(59, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}
void decodebind64(__axiom_decimal64 *__x, const __axiom_decimal64 *__enc) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__enc);
    (void)__axc_dfn(59, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}
void decodebind128(__axiom_decimal128 *__x, const __axiom_decimal128 *__enc) {
    __axc_dw x, r;
    __AXC_DFP_IN(x, *__enc);
    (void)__axc_dfn(59, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}

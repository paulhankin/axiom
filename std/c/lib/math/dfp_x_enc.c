/* dfp_x_enc.c - the decimal floating functions of <math.h> (C23 7.12) encodedec, decodedec, encodebin, decodebin
   for _Decimal32, _Decimal64 and _Decimal128 (the encodings in unsigned char
   arrays, in memory's byte order): each calls __axc_dfn
   (std/c/rt/dfp.ax) with its operands' encodings (axc/dfp.h). */
#include "axc/dfp.h"
/* (C23 7.12.16: the encodings in arrays of unsigned char, of any alignment) */
static void copy_bytes(void *d, const void *s, int n)
{
    unsigned char *dp = (unsigned char *)d;
    const unsigned char *sp = (const unsigned char *)s;
    int i;
    for (i = 0; i < n; i++)
        dp[i] = sp[i];
}

/* encodedec */
void encodedecd32(unsigned char *restrict __enc, const __axiom_decimal32 *restrict __x) {
    __axc_dw x, r;
    __axiom_decimal32 t;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(56, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(&t, r);
    copy_bytes(__enc, &t, 4);
}
void encodedecd64(unsigned char *restrict __enc, const __axiom_decimal64 *restrict __x) {
    __axc_dw x, r;
    __axiom_decimal64 t;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(56, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(&t, r);
    copy_bytes(__enc, &t, 8);
}
void encodedecd128(unsigned char *restrict __enc, const __axiom_decimal128 *restrict __x) {
    __axc_dw x, r;
    __axiom_decimal128 t;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(56, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(&t, r);
    copy_bytes(__enc, &t, 16);
}

/* decodedec */
void decodedecd32(__axiom_decimal32 *restrict __x, const unsigned char *restrict __enc) {
    __axc_dw x, r;
    __axiom_decimal32 t;
    copy_bytes(&t, __enc, 4);
    __AXC_DFP_IN(x, t);
    (void)__axc_dfn(57, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}
void decodedecd64(__axiom_decimal64 *restrict __x, const unsigned char *restrict __enc) {
    __axc_dw x, r;
    __axiom_decimal64 t;
    copy_bytes(&t, __enc, 8);
    __AXC_DFP_IN(x, t);
    (void)__axc_dfn(57, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}
void decodedecd128(__axiom_decimal128 *restrict __x, const unsigned char *restrict __enc) {
    __axc_dw x, r;
    __axiom_decimal128 t;
    copy_bytes(&t, __enc, 16);
    __AXC_DFP_IN(x, t);
    (void)__axc_dfn(57, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}

/* encodebin */
void encodebind32(unsigned char *restrict __enc, const __axiom_decimal32 *restrict __x) {
    __axc_dw x, r;
    __axiom_decimal32 t;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(58, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(&t, r);
    copy_bytes(__enc, &t, 4);
}
void encodebind64(unsigned char *restrict __enc, const __axiom_decimal64 *restrict __x) {
    __axc_dw x, r;
    __axiom_decimal64 t;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(58, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(&t, r);
    copy_bytes(__enc, &t, 8);
}
void encodebind128(unsigned char *restrict __enc, const __axiom_decimal128 *restrict __x) {
    __axc_dw x, r;
    __axiom_decimal128 t;
    __AXC_DFP_IN(x, *__x);
    (void)__axc_dfn(58, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(&t, r);
    copy_bytes(__enc, &t, 16);
}

/* decodebin */
void decodebind32(__axiom_decimal32 *restrict __x, const unsigned char *restrict __enc) {
    __axc_dw x, r;
    __axiom_decimal32 t;
    copy_bytes(&t, __enc, 4);
    __AXC_DFP_IN(x, t);
    (void)__axc_dfn(59, 0, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}
void decodebind64(__axiom_decimal64 *restrict __x, const unsigned char *restrict __enc) {
    __axc_dw x, r;
    __axiom_decimal64 t;
    copy_bytes(&t, __enc, 8);
    __AXC_DFP_IN(x, t);
    (void)__axc_dfn(59, 1, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}
void decodebind128(__axiom_decimal128 *restrict __x, const unsigned char *restrict __enc) {
    __axc_dw x, r;
    __axiom_decimal128 t;
    copy_bytes(&t, __enc, 16);
    __AXC_DFP_IN(x, t);
    (void)__axc_dfn(59, 2, &r, &x, 0, 0, 0);
    __AXC_DFP_STORE(__x, r);
}

/* The static rounding directions of #pragma STDC FENV_ROUND and
   FENV_DEC_ROUND (C23 7.6.2, 7.6.3) for Annex H's functions of their
   tables (H.10p4-5): the narrowing operations (f32addf64, d32addd64x), the
   encoding conversions (f32encf64, d32encbind64) and the string
   conversions of encodings (strtoencf32, strtoencbind32); a call of each
   in the pragma's scope rounds in its direction, one outside it to
   nearest. */
/* flags: --c-std=c23 */
/* native-only */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <fenv.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void hex(const unsigned char *p, int n) {
    for (int i = n - 1; i >= 0; i--)
        printf("%02x", p[i]);
    printf(" ");
}

static void up(unsigned char *e4, const unsigned char *e8, const unsigned char *d8) {
#pragma STDC FENV_ROUND FE_UPWARD
#pragma STDC FENV_DEC_ROUND FE_DEC_UPWARD
    volatile _Float64 one = 1, tiny = 1e-30;
    printf("%a ", (double)f32addf64(one, tiny));
    f32encf64(e4, e8); hex(e4, 4);
    strtoencf32(e4, "1.00000001", 0); hex(e4, 4);
    printf("%Ha ", d32addd64x(1, 1E-20d64x));
    d32encbind64(e4, d8); hex(e4, 4);
    strtoencbind32(e4, "0.12345671", 0); hex(e4, 4);
    printf("\n");
}

static void near(unsigned char *e4, const unsigned char *e8, const unsigned char *d8) {
    volatile _Float64 one = 1, tiny = 1e-30;
    printf("%a ", (double)f32addf64(one, tiny));
    f32encf64(e4, e8); hex(e4, 4);
    strtoencf32(e4, "1.00000001", 0); hex(e4, 4);
    printf("%Ha ", d32addd64x(1, 1E-20d64x));
    d32encbind64(e4, d8); hex(e4, 4);
    strtoencbind32(e4, "0.12345671", 0); hex(e4, 4);
    printf("\n");
}

int main(void) {
    unsigned char e4[4], e8[8], d8[8];
    strtoencf64(e8, "1.00000001", 0);
    strtoencbind64(d8, "0.12345671", 0);
    up(e4, e8, d8);
/* expect: 0x1.000002p+0 3f800001 3f800001 1.000001 2f12d688 2f12d688  */
    near(e4, e8, d8);
/* expect: 0x1p+0 3f800000 3f800000 1.000000 2f12d687 2f12d687  */
    return 0;
}

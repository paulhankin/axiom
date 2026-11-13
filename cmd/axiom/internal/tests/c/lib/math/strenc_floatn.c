/* The string conversions of the interchange formats' encodings (C23 Annex
   H.12.4, H.12.5): strfromencfN, strtoencfN and wcstoencfN of binary16 ...
   binary128, and strfromencdecdN, strfromencbindN, strtoencdecdN,
   strtoencbindN, wcstoencdecdN and wcstoencbindN of decimal32 ...
   decimal128 (densely packed decimal, binary integer decimal), the
   encodings' bytes little-endian (__STDC_ENDIAN_NATIVE__); their values
   rounded as strtofN's and strtodN's are, in each rounding direction. */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <errno.h>
#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

static void hex(const unsigned char *p, int n) {
    for (int i = n - 1; i >= 0; i--)
        printf("%02x", p[i]);
    printf(" ");
}

int main(void) {
    unsigned char e2[2], e4[4], e8[8], e16[16];
    char b[64], *end;
    wchar_t *wend;
    strtoencf16(e2, "1.5", 0); hex(e2, 2);
    strtoencf32(e4, "0.1", &end); hex(e4, 4);
    strtoencf64(e8, "-2.5e-3x", &end); hex(e8, 8); printf("%s ", end);
    wcstoencf128(e16, L"1e4932", &wend); hex(e16, 16);
    printf("%d\n", (int)(*wend == 0));
/* expect: 3e00 3dcccccd bf647ae147ae147b x 7ffeae596552b8fded99d037e3d04b75 1 */
    strfromencf16(b, sizeof b, "%a", e2); printf("%s ", b);
    strfromencf32(b, sizeof b, "%.9g", e4); printf("%s ", b);
    strfromencf64(b, sizeof b, "%e", e8); printf("%s ", b);
    strfromencf128(b, sizeof b, "%g", e16); printf("%s\n", b);
/* expect: 0x1.8p+0 0.100000001 -2.500000e-03 1e+4932 */
    /* decimal: the same value in its two encodings */
    strtoencdecd32(e4, "1.5", 0); hex(e4, 4);
    strtoencbind32(e4, "1.5", 0); hex(e4, 4);
    strtoencdecd64(e8, "-7.50", 0); hex(e8, 8);
    strtoencbind64(e8, "-7.50", 0); hex(e8, 8);
    wcstoencbind128(e16, L"123E-2", &wend); hex(e16, 16);
    printf("\n");
/* expect: 22400015 3200000f a2300000000003d0 b1800000000002ee 303c000000000000000000000000007b  */
    strfromencbind128(b, sizeof b, "%a", e16); printf("%s ", b);
    strtoencdecd128(e16, "0.000001", 0); strfromencdecd128(b, sizeof b, "%a", e16); printf("%s ", b);
    strtoencdecd64(e8, "1e-500", 0); strfromencdecd64(b, sizeof b, "%a", e8); printf("%s ", b);
    wcstoencdecd32(e4, L"9.9999999", 0); strfromencdecd32(b, sizeof b, "%.3e", e4); printf("%s\n", b);
/* expect: 1.23 0.000001 0e-398 1.000e+01 */
    /* the directions: binary's for binary encodings, decimal's for decimal ones */
    fesetround(FE_UPWARD);
    fe_dec_setround(FE_DEC_DOWNWARD);
    strtoencf32(e4, "0.1", 0); hex(e4, 4);
    strtoencbind32(e4, "0.12345678", 0); hex(e4, 4);
    errno = 0;
    strtoencf16(e2, "1e10", 0); hex(e2, 2); printf("%d\n", errno == ERANGE);
/* expect: 3dcccccd 2f12d687 7c00 1 */
    fesetround(FE_TONEAREST);
    fe_dec_setround(FE_DEC_TONEAREST);
    return 0;
}

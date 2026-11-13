/* C23 Annex H.11.3's encoding functions: encodefN and decodefN (the
   representation's bytes, little-endian), fMencfN (a conversion between
   the encodings, rounded), dMencdecdN and dMencbindN. */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include "print.h"
#include <math.h>
#include <string.h>

int main(void)
{
    unsigned char b16[3], b32[4], b64[8], b128[16], d32[4], d64[9];
    _Float16 h = 1.5F16;
    _Float32 f;
    _Float64 d;
    _Float128 q = 1.0F128 / 3.0F128, q2;
    _Decimal64 x = 1.25dd, y;
    _Decimal32 z;
    encodef16(b16 + 1, &h); /* (any alignment) */
    print_u64(b16[1] | (b16[2] << 8));
    f32encf16(b32, b16 + 1);
    decodef32(&f, b32);
    print_f64(f);
    encodef128(b128, &q);
    f64encf128(b64, b128);
    decodef64(&d, b64);
    print_bool(d == (_Float64)q);
    f16encf64(b16, b64);
    decodef16(&h, b16);
    print_f64(h);
    decodef128(&q2, b128);
    print_bool(memcmp(&q2, &q, 16) == 0);
    encodedecd64(d64 + 1, &x);
    d32encdecd64(d32, d64 + 1);
    decodedecd32(&z, d32);
    print_bool(z == 1.25df);
    encodebind64(d64, &x);
    d32encbind64(d32, d64);
    decodebind32(&z, d32);
    d64encbind32(d64, d32);
    decodebind64(&y, d64);
    print_bool(y == 1.25dd);
    return 0;
}
/* expect: 15872 */
/* expect: 1.5 */
/* expect: true */
/* expect: 0.333251953125 */
/* expect: true */
/* expect: true */
/* expect: true */

/* flags: --c-std=c23 */
/* The interchange and extended floating types (C23 Annex H): _Float16,
   _Float32, _Float64, _Float128, _Float32x, _Float64x; their constants'
   suffixes, arithmetic (binary16's rounded to it), conversions, the usual
   arithmetic conversions (a type of more values, then interchange over
   standard over extended), <float.h>'s macros, and arguments and
   results. */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <float.h>
#include "host.h"

#define KIND(x) _Generic((x), _Float16: 16, float: 1, _Float32: 32, double: 2, _Float64: 64, long double: 3, \
                         _Float128: 128, _Float32x: 320, _Float64x: 640, default: 0)

[[gnu::noinline]] static _Float16 hmul(_Float16 a, _Float16 b) { return a * b; }
[[gnu::noinline]] static _Float128 qdiv(_Float128 a, _Float128 b) { return a / b; }
[[gnu::noinline]] static _Float64x xadd(_Float64x a, int b) { return a + b; }
struct hq { _Float16 h; _Float128 q; };
[[gnu::noinline]] static struct hq both(struct hq s) { s.h += 1; s.q *= 2; return s; }

int main(void) {
    _Float16 h = 0.1f16;
    print_f64((double)h);
/* expect: 0.0999755859375 */
    print_f64((double)(h + 0.2F16));
/* expect: 0.2998046875 */
    print_f64((double)hmul(300.0f16, 300.0f16));
/* expect: Infinity */
    print_f64((double)(_Float16)65519.0);
/* expect: 65504 */
    print_f64((double)(_Float16)6e-8 * 0x1p24);
/* expect: 1 */
    print_i64((int)(_Float16)-3.75f16);
/* expect: -3 */
    print_f64((double)(_Float16)12345);
/* expect: 12344 */
    _Float32 f = 1.0f32 / 3;
    print_f64((double)f);
/* expect: 0.3333333432674408 */
    _Float64 d = 2.0F64 / 3;
    print_f64(d);
/* expect: 0.6666666666666666 */
    _Float128 q = qdiv(1, 3);
    print_i64((long long)(q * 3e18f128) - 999999999999999999LL);
/* expect: 1 */
    print_i64(q * 3 == 1);
/* expect: 1 */
    print_i64((double)q * 3 == 1.0);
/* expect: 1 */
    _Float128 big = 1e4000f128;
    print_i64(big > (_Float128)DBL_MAX);
/* expect: 1 */
    print_i64(xadd(0.5f64x, 2) == 2.5);
/* expect: 1 */
    _Float32x x32 = 1.5f32x;
    print_f64((double)x32);
/* expect: 1.5 */
    struct hq s = both((struct hq){1.5f16, 0.25f128});
    print_f64((double)s.h + (double)s.q);
/* expect: 3 */
    print_i64(sizeof(_Float16) + sizeof(_Float32) * 10 + sizeof(_Float64) * 100 + sizeof(_Float128) * 1000);
/* expect: 16842 */
    print_i64(KIND(h + h) + KIND(h + 1.0f) * 1000);
/* expect: 1016 */
    print_i64(KIND(1.0f + 1.0f32));
/* expect: 32 */
    print_i64(KIND(1.0 + 1.0f64));
/* expect: 64 */
    print_i64(KIND(1.0f32 + 1.0f64));
/* expect: 64 */
    print_i64(KIND(1.0f32x + 1.0));
/* expect: 2 */
    print_i64(KIND(1.0f128 + 1.0f64x));
/* expect: 128 */
    print_i64(KIND(h * 2));
/* expect: 16 */
    print_i64(FLT16_MANT_DIG * 1000 + FLT128_MANT_DIG);
/* expect: 11113 */
    print_f64((double)FLT16_MAX + (double)FLT16_MIN * 0);
/* expect: 65504 */
    print_f64((double)FLT16_EPSILON);
/* expect: 0.0009765625 */
    print_i64(FLT128_MAX_EXP + FLT128_DIG);
/* expect: 16417 */
    print_i64(FLT32_DIG + FLT64_DIG * 100);
/* expect: 1506 */
    print_i64(FLT64X_MANT_DIG >= 64);
/* expect: 1 */
    unsigned long long u = (unsigned long long)(_Float128)18446744073709551615.0f128;
    print_u64(u);
/* expect: 18446744073709551615 */
    _Float128 fromint = (_Float128)0x7fffffffffffffffLL;
    print_i64(fromint == 9223372036854775807.0f128);
/* expect: 1 */
    print_i64((long long)(_Float16)-0.0f16 == 0 && 1 / (double)(_Float16)-0.0f16 < 0);
/* expect: 1 */
    return 0;
}

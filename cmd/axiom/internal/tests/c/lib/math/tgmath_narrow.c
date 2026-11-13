/* <tgmath.h>'s macros of the narrowing operations with an fN, fNx, dN or
   dNx prefix (C23 7.27p11, H.13p5: Tables H.9 and H.10's examples): the
   function of the arguments' type P where there is one, else of the first
   type the prefix lists whose values include P's; and the macros of the
   quantum functions (7.27p12). */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <stdio.h>
#include <tgmath.h>

#define T(x) _Generic((x), _Float16: "f16", _Float128: "f128", _Float32: "f32", _Float64: "f64", \
                      _Float32x: "f32x", _Float64x: "f64x", float: "float", double: "double", \
                      long double: "ld", _Decimal32: "d32", _Decimal64: "d64", _Decimal64x: "d64x", \
                      _Decimal128: "d128", long long: "ll", bool: "bool", default: "?")

int main(void) {
    int n = 2;
    double d = 2.5;
    _Float32 f32 = 1.5f32;
    _Float32x f32x = 2.25f32x;
    _Float64 f64 = 0.5f64;
    _Float64x f64x = 3.0f64x;
    _Float128 f128 = 4.0f128;
    /* the result types, and the values (one rounding of the exact result) */
    printf("%s %s %s %s %s\n", T(f32add(f64x, f64)), T(f32xsqrt(n)), T(f32fma(f32x, n, f32x)), T(f32add(f32, f32)),
           T(f64div(f32x, f32x)));
/* expect: f32 f32x f32 f32 f64 */
    printf("%g %g %g %g %g\n", (double)f32add(f64x, f64), (double)f32xsqrt(n), (double)f32fma(f32x, n, f32x),
           (double)f32add(f32, f32), (double)f64div(f32x, f32x));
/* expect: 3.5 1.41421 6.75 3 1 */
    printf("%s %s %s %g\n", T(f16mul(f128, d)), T(f64xsub(f128, 1)), T(f32mul(f128, f32x)), (double)f16mul(f128, d));
/* expect: f16 f64x f32 10 */
    /* the decimal prefixes: C23's d32 and d64 (the d128 function for a
       _Decimal128 or with d64, else the d64 one), and d64x */
    _Decimal32 d32 = 1.5df;
    _Decimal64 d64 = 2.5dd;
    _Decimal64x x = 0.25d64x;
    _Decimal128 d128 = 4.0dl;
    printf("%s %s %s %s %s\n", T(d32sub(d32, d128)), T(d32div(d64, n)), T(d64fma(d32, d64, d128)), T(d64add(d32, d32)),
           T(d32add(x, x)));
/* expect: d32 d32 d64 d64 d32 */
    printf("%Ha %Ha %Da %Da %Ha\n", d32sub(d32, d128), d32div(d64, n), d64fma(d32, d64, d128), d64add(d32, d32),
           d32add(x, x));
/* expect: -2.5 1.25 7.75 3.0 0.50 */
    printf("%s %s %DDa\n", T(d64xadd(d64, d32)), T(d64xsqrt(n)), (_Decimal128)d64xmul(x, d128));
/* expect: d64x d64x 1.000 */
    /* quantize, samequantum, quantum, llquantexp */
    printf("%s %s %s %s ", T(quantize(d64, 1.00dd)), T(samequantum(d32, d32)), T(quantum(x)), T(llquantexp(d128)));
    printf("%Da %d %DDa %lld\n", quantize(d64, 1.00dd), (int)samequantum(d32, 1.0df), (_Decimal128)quantum(x),
           llquantexp(d128));
/* expect: d64 bool d64x ll 2.50 1 0.01 -1 */
    return 0;
}

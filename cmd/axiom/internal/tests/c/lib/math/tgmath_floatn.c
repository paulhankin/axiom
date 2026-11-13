/* <tgmath.h> with arguments of the interchange and extended floating types
   (C23 H.11): the function of their common real type, an integer argument
   counted as double (as _Float32x with an extended one, H.13p3); each type's own functions (sqrtf16 ... sqrtf64x),
   whose results have the type. */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>

#define T(x) _Generic((x), _Float16: "f16", _Float128: "f128", _Float32: "f32", _Float64: "f64", \
                      _Float32x: "f32x", _Float64x: "f64x", float: "float", double: "double", \
                      long double: "long double", _Float32 _Complex: "cf32", default: "?")

int main(void) {
    char b[64];
    _Float32 _Complex zc = 1;
    printf("%s %s %s %s\n", T(sqrt((_Float16)2)), T(sqrt((_Float128)2)), T(sqrt((_Float32)2)), T(sqrt((_Float64)2)));
    printf("%s %s %s\n", T(pow((_Float16)2, 3)), T(fmax((_Float16)1, (_Float128)2)), T(atan2((_Float16)1, 2.0f)));
    printf("%s %s %s %s\n", T(sqrt((_Float32x)2)), T(exp((_Float64x)1)), T(pow((_Float32)2, (_Float32x)3)),
           T(exp(zc)));
    /* (H.13p3: an integer counts as _Float32x with an extended argument;
       H.9: double with _Float32x, of the same values, is double) */
    printf("%s %s %s\n", T(pow((_Float32x)2, 3)), T(pow(2.0, (_Float32x)3)), T(fmax(1, (_Float64x)2)));
    strfromf128(b, sizeof b, "%.33g", sqrt((_Float128)2));
    printf("%s %g\n", b, (double)sqrt((_Float16)2));
    return 0;
}
// expect: f16 f128 f32 f64
// expect: double f128 float
// expect: f32x f64x f32x cf32
// expect: f32x double f64x
// expect: 1.4142135623730950488016887242097 1.41406

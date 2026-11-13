/* <tgmath.h> with arguments of the interchange and extended floating types
   (C23 H.11): the function of their common real type, an integer argument
   counted as double; _Float16's and _Float128's own functions (f16, f128),
   and for _Float32, _Float64 and _Float32x the standard function of their
   format. */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <stdio.h>
#include <stdlib.h>
#include <tgmath.h>

#define T(x) _Generic((x), _Float16: "f16", _Float128: "f128", float: "float", double: "double", \
                      long double: "long double", default: "?")

int main(void) {
    char b[64];
    printf("%s %s %s %s\n", T(sqrt((_Float16)2)), T(sqrt((_Float128)2)), T(sqrt((_Float32)2)), T(sqrt((_Float64)2)));
    printf("%s %s %s\n", T(pow((_Float16)2, 3)), T(fmax((_Float16)1, (_Float128)2)), T(atan2((_Float16)1, 2.0f)));
    strfromf128(b, sizeof b, "%.33g", sqrt((_Float128)2));
    printf("%s %g\n", b, (double)sqrt((_Float16)2));
    return 0;
}
// expect: f16 f128 float double
// expect: double f128 float
// expect: 1.4142135623730950488016887242097 1.41406

/* <tgmath.h> with decimal floating arguments (C23 7.27p3): the function of
   the widest decimal type among them, _Decimal64 for an integer with a
   _Decimal32; with Annex H's _Decimal64x (H.13p3), _Decimal128 above it and
   an integer counted as _Decimal64x. */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <stdio.h>
#include <tgmath.h>

#define T(x) _Generic((x), _Decimal32: "d32", _Decimal64: "d64", _Decimal128: "d128", _Decimal64x: "d64x", \
                      double: "double", default: "?")

int main(void) {
    printf("%s %s %s %s ", T(sqrt(2.0df)), T(pow(2.0df, 3)), T(fmax(1.0df, 2.0dd)), T(atan2(1.0dl, 2.0df)));
    printf("%s %s %s\n", T(exp(1.0dd)), T(fabs(-1.5df)), T(nextafter(1.0dd, 2.0dd)));
/* expect: d32 d64 d64 d128 d64 d32 d64 */
    printf("%Ha %Da %DDa %Da\n", sqrt(1.44df), pow(2.0df, 10), cbrt(8.dl), fma(2.0dd, 3, 1));
/* expect: 1.2 1024.0000000000 2 7.0 */
    printf("%s %s %s %s\n", T(sqrt(2.0d64x)), T(pow(2.0df, 3.0d64x)), T(fmax(1.0d64x, 2.0dl)), T(exp(1.0d64x)));
/* expect: d64x d64x d128 d64x */
    return 0;
}

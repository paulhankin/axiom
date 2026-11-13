/* flags: --c-std=gnu17 */
/* The GNU dialects' __float128 (_Float128) and its q suffix, the f16 and
   f128 builtins, GNU imaginary constants (2.0i), and the mode attribute's
   floating (TF: binary128, long double where that is its format), 128-bit
   integer and vector modes. __real__ and __imag__ are not supported. */
#include "host.h"

typedef float hf __attribute__((mode(HF)));
typedef float tf __attribute__((mode(TF)));
typedef int ti __attribute__((mode(TI)));
typedef unsigned uti __attribute__((mode(TI)));
typedef int v4si __attribute__((mode(V4SI)));
typedef float v2df __attribute__((mode(V2DF)));

int main(void) {
    __float128 q = 1.0q / 3;
    print_i64(_Generic(q, _Float128: 1, default: 0) + _Generic((tf)0, _Float128: 10, long double: 10, default: 0) +
              _Generic((hf)0, _Float16: 100, default: 0));
/* expect: 111 */
    print_i64(q * 3 == 1.0Q);
/* expect: 1 */
    print_f64((double)__builtin_fabsf128(-2.5q) + (double)__builtin_copysignf16(1.5f16, -1.0f16));
/* expect: 1 */
    print_i64(__builtin_isinf(__builtin_inff128()) + __builtin_isnan(__builtin_nanf16("")) * 10);
/* expect: 11 */
    print_f64((double)__builtin_huge_valf128() > 1e308);
/* expect: 1 */
    ti big = (ti)1 << 100;
    uti ubig = (uti)big * 4;
    print_i64(sizeof(ti) + (long long)(ubig >> 100) * 100);
/* expect: 416 */
    v4si v = {1, 2, 3, 4};
    v = v * v;
    print_i64(v[3] + sizeof(v4si) * 100);
/* expect: 1616 */
    v2df d = {1.5, 2.5};
    print_f64(d[0] + d[1]);
/* expect: 4 */
    _Complex double z = 2.0i + 1.0;
    print_f64(__builtin_creal(z) + __builtin_cimag(z) * 10);
/* expect: 21 */
    return 0;
}

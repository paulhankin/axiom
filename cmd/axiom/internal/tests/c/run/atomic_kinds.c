/* flags: --c-std=gnu23 */
/* Atomic objects of the newer type kinds (6.2.5p27): decimal floating
   types, _Float128, _Float32 and _Float64x, imaginary types, and vectors
   of 8, 16 and 32 bytes (a vector's value is in memory, as a structure's:
   cgen_atomic.ax, is_mem), loaded, stored, exchanged and updated by
   compound assignments; each atomic type is as large as its non-atomic one
   here (none needs padding to a power of two). */
#include <stdatomic.h>
#include "host.h"
typedef int v4si __attribute__((vector_size(16)));
typedef short v4hi __attribute__((vector_size(8)));
typedef int v8si __attribute__((vector_size(32)));
_Atomic _Decimal64 d64 = 1.5DD;
_Atomic _Decimal32 d32 = 2.5DF;
_Atomic _Decimal128 d128 = 3.25DL;
_Atomic _Float128 q = 1.25;
_Atomic v4si v = {1, 2, 3, 4};
_Atomic v4hi vh = {1, 2, 3, 4};
_Atomic v8si v8 = {1, 2, 3, 4, 5, 6, 7, 8};
_Atomic _Float32 f32 = 0.5f;
_Atomic _Float64x f64x = 2.0;
_Atomic double _Imaginary im;
#define SAME(T) _Static_assert(sizeof(_Atomic T) == sizeof(T), #T)
SAME(_Decimal32); SAME(_Decimal64); SAME(_Decimal128); SAME(_Float128); SAME(v4si); SAME(v4hi); SAME(v8si);
SAME(double _Imaginary);
int main(void) {
    d64 += 2.25DD;
    d32 *= 4.0DF;
    d128 -= 0.25DL;
    print_i64((long long)(d64 * 100));
    print_i64((long long)(d32 * 10));
    print_i64((long long)(d128 * 100));
    q += 2.5;
    q *= 2;
    print_i64((long long)(q * 100));
    v4si a = v;
    a += 10;
    v = a;
    v4si nv = {5, 6, 7, 8};
    v4si b = atomic_exchange(&v, nv);
    print_i64(b[0] * 1000 + b[3] * 10 + v[2]);
    v4si e = {5, 6, 7, 8}, d = {9, 9, 9, 9};
    print_i64(atomic_compare_exchange_strong(&v, &e, d) * 10 + v[1]);
    vh = vh + vh;
    print_i64(vh[3]);
    v8si c = v8;
    c *= 2;
    v8 = c;
    v8si o8 = atomic_exchange(&v8, c + 1);
    print_i64(o8[7] * 100 + v8[7]);
    f32 += 1.0f;
    f64x *= 3;
    print_i64((long long)(f32 * 10) * 100 + (long long)f64x);
    im = 2.0 * 1.0i;
    im += 1.5 * 1.0i;
    print_i64((long long)(__builtin_cimag(im + 0.0) * 10));
    return 0;
}
/* expect: 375 */
/* expect: 100 */
/* expect: 300 */
/* expect: 750 */
/* expect: 11147 */
/* expect: 19 */
/* expect: 8 */
/* expect: 1617 */
/* expect: 1506 */
/* expect: 35 */

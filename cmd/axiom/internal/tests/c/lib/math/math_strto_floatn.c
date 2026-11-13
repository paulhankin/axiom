/* C23 Annex H.12's numeric conversions of the interchange and extended
   binary types: strtofN, strfromfN and wcstofN, of every type, with the
   type's results. */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include "print.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define T(e, R) _Generic((e), R: 1, default: 0)

int main(void)
{
    char b[64];
    wchar_t *e;
    const wchar_t *w = L"  0x1.8p1 rest";
    print_bool(T(strtof32("1.5", 0), _Float32) && T(strtof64("1.5", 0), _Float64) && T(strtof32x("1.5", 0), _Float32x) &&
               T(strtof64x("1.5", 0), _Float64x) && T(wcstof16(w, 0), _Float16) && T(wcstof128(w, 0), _Float128));
    print_f64(strtof32("0.1", 0));
    print_f64(strtof64x("1e-4950", 0) > 0 ? 1.0 : 0.0);
    strfromf64x(b, sizeof b, "%.20g", strtof64x("0.1", 0));
    print_cstr(b);
    strfromf32x(b, sizeof b, "%a", 3.0F32x);
    print_cstr(b);
    print_f64(wcstof16(w, &e));
    print_bool(e == w + 9);
    print_f64((double)wcstof128(L"1e4932", &e));
    print_f64(wcstof32(L"2.5", 0) + wcstof64(L"0.25", 0) + wcstof32x(L"0.125", 0) + wcstof64x(L"1", 0));
    /* (H.11's evaluation types, FLT_EVAL_METHOD 0) */
    print_bool(T((_Float16_t)0, float) && T((_Float32_t)0, _Float32) && T((_Float64_t)0, _Float64) &&
               T((_Float128_t)0, _Float128) && T((long_double_t)0, long double));
    return 0;
}
/* expect: true */
/* expect: 0.10000000149011612 */
/* expect: 1 */
/* expect: 0.1 */
/* expect: 0x1.8p+1 */
/* expect: 3 */
/* expect: true */
/* expect: inf */
/* expect: 3.875 */
/* expect: true */

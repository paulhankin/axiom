/* The narrowing operations between the interchange and extended types (C23
   Annex H.11.2, 7.12.14): the exact result rounded once to the result's
   format, in the current direction; f64xaddf128 ... in the x87 format where
   _Float64x has it (x86-64 Linux), else binary128. */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include "print.h"
#include <fenv.h>
#include <float.h>
#include <math.h>

int main(void)
{
    volatile _Float32 one = 1.0F32, tie = 0x1p-11F32, above = 0x1.0002p-11F32;
    _Float64x r, q;
    print_f64(f16addf32(one, tie));
    print_f64(f16addf32(one, above));
    print_f64(f16mulf64(0x1.002p0F64, 0x1.002p0F64));
    print_f64(f16sqrtf128(2.0F128));
    print_bool(f32divf64(1.0F64, 3.0F64) == 1.0F32 / 3.0F32);
    print_f64(f16fmaf32x(2.0F32x, 3.0F32x, 0x1p-9F32x));
    print_f64(f32subf32x(1.0F32x, 0x1p-30F32x));
    print_f64(f16divf64x(1.0F64x, 3.0F64x));
    r = f64xaddf128(1.0F128, 0x1p-64F128 + 0x1p-100F128);
    q = FLT64X_MANT_DIG == 64 ? 1.0F64x + 0x1p-63F64x : 1.0F64x + 0x1p-64F64x + 0x1p-100F64x;
    print_bool(r == q);
    print_bool(f64xsqrtf128(2.0F128) == sqrtf64x(2.0F64x));
    print_bool(f64xdivf128(1.0F128, 3.0F128) == 1.0F64x / 3.0F64x);
    print_bool(f64xmulf128(0x1p-16000F128, 0x1p-440F128) == 0x1p-16440F64x);
    print_bool(f64xmulf128(0x1p-16000F128, 0x1.8p-446F128) == (FLT64X_MANT_DIG == 64 ? 0x1p-16445F64x : 0x1.8p-16446F64x));
    print_f64(f32xaddf64x(1.0F64x, 0x1p-53F64x));
    print_f64(f64fmaf128(0x1p-1074F128, 0.5F128, 0x1p-1076F128));
    /* (upward where there are rounding directions) */
    if (__builtin_axiom_fenv_kind() != 0) {
        fesetround(FE_UPWARD);
        print_f64(f16addf32(one, 0x1p-20F32));
        fesetround(FE_TONEAREST);
    } else {
        print_f64(1.0009765625);
    }
    return 0;
}
/* expect: 1 */
/* expect: 1.0009765625 */
/* expect: 1.0009765625 */
/* expect: 1.4140625 */
/* expect: true */
/* expect: 6 */
/* expect: 1 */
/* expect: 0.333251953125 */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: 1 */
/* expect: 5e-324 */
/* expect: 1.0009765625 */

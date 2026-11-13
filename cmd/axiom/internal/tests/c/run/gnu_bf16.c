/* flags: --c-std=gnu23 */
/* __bf16 (a GNU dialect): bfloat16, binary32's sign and exponent with 7
   fraction bits. Conversions round once to nearest, ties to even, from the
   source's own value (float, double, long double, integers, decimal);
   arithmetic is float's, rounded to __bf16 after each operation;
   subnormals, infinities and NaNs; static initializers (folded the same
   way), the characteristic macros, _Generic, atomics, arguments and
   results. (Every rounding direction: c/lib/math/bf16_convert.c.) */
#include "host.h"

static unsigned bits(__bf16 x) {
    unsigned short b;
    __builtin_memcpy(&b, &x, 2);
    return b;
}

static __bf16 g[] = { 1.5, -2, 1e-40, 3.14159265358979, 1e39, 0x1.01p0, 0x1.03p0 };
static __bf16 mad(__bf16 a, __bf16 b, __bf16 c) { return a * b + c; }
static _Atomic __bf16 acc = 1;

int main(void) {
    for (int i = 0; i < 7; i++)
        print_u64(bits(g[i]));
    /* the same at run time */
    volatile double vd[] = { 1.5, -2, 1e-40, 3.14159265358979, 1e39, 0x1.01p0, 0x1.03p0 };
    unsigned s = 0;
    for (int i = 0; i < 7; i++)
        s += bits((__bf16)vd[i]) == bits(g[i]);
    print_i64(s);
    /* once from double: just above the midpoint of 1 and 1 + 2^-7 */
    volatile double up = 0x1.0100000001p0;
    print_u64(bits((__bf16)up));
    volatile float fl = 0x1.0100000p0f;  /* the midpoint: to even */
    print_u64(bits((__bf16)fl));
    /* integers, rounded once (2^60 + 2^52 + 1 is above a midpoint) */
    volatile long long n = (1LL << 60) + (1LL << 52) + 1;
    print_u64(bits((__bf16)n));
    print_u64(bits((__bf16)((1LL << 60) + (1LL << 52) + 1)));
    print_i64((long long)(__bf16)n);
    volatile long double ld = 1.0L + 0x1p-8L + 0x1p-40L;
    print_u64(bits((__bf16)ld));
    print_i64((long double)(__bf16)ld == 1.0078125L);
    volatile _Decimal64 dd = 1.00390625000001DD;
    print_u64(bits((__bf16)dd));
    print_i64((_Decimal64)(__bf16)dd == 1.0078125DD);
    /* arithmetic */
    __bf16 a = 1.0f / 3.0f, b = 2;
    print_f64((double)(a + b));
    print_f64((double)mad(a, b, 0.25));
    __bf16 big = 3e38;
    print_u64(bits(big * 2));
    print_i64(a < b);
    print_i64((__bf16)(0.0f / 0.0f) != (__bf16)(0.0f / 0.0f));
    print_u64(bits(-a));
    /* to and from _Float16 through float, exact both ways where it can be */
    print_f64((double)(_Float16)(__bf16)3.0f);
    print_i64(_Generic(a, __bf16: 1, _Float16: 2, float: 3, default: 4));
    print_i64(_Generic(a + 1.0f, float: 1, default: 2));
    print_i64(sizeof(__bf16) * 10 + _Alignof(__bf16));
    acc += 2;
    print_f64((double)acc);
    print_f64((double)__BFLT16_MAX__);
    print_f64((double)__BFLT16_EPSILON__);
    print_i64(__BFLT16_MANT_DIG__ * 1000 + __BFLT16_MAX_EXP__);
    return 0;
}
/* expect: 16320 */
/* expect: 49152 */
/* expect: 1 */
/* expect: 16457 */
/* expect: 32640 */
/* expect: 16256 */
/* expect: 16258 */
/* expect: 7 */
/* expect: 16257 */
/* expect: 16256 */
/* expect: 23937 */
/* expect: 23937 */
/* expect: 1161928703861587968 */
/* expect: 16257 */
/* expect: 1 */
/* expect: 16257 */
/* expect: 1 */
/* expect: 2.328125 */
/* expect: 0.91796875 */
/* expect: 32640 */
/* expect: 1 */
/* expect: 1 */
/* expect: 48811 */
/* expect: 3 */
/* expect: 1 */
/* expect: 1 */
/* expect: 22 */
/* expect: 3 */
/* expect: 3.3895313892515355e+38 */
/* expect: 0.0078125 */
/* expect: 8128 */

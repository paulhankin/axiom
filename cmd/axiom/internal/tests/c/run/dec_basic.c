/* The decimal floating types (C23 6.2.5, 6.4.4.2, 6.3.1, 6.5): constants
   with their quanta, the operators (the preferred quanta of IEEE 754-2019
   5.2), conversions with integers of every width and the binary floating
   types, sizes, _Generic, ++ and --, compound assignment, ?:, static
   initializers and constexpr objects (folded by the compiler), the truth
   of a value of any cohort. */
/* flags: --c-std=c23 */
#include "host.h"
#include "../dec_show.h"

static _Decimal64 st_sum = 1.10dd + 2.205dd;   /* folded */
static _Decimal32 st_conv = 7;                  /* an integer to _Decimal32 */
static _Decimal128 st_q = 1.000dl / 4;
static const _Decimal64 st_tab[] = {1.5dd, -0.0dd, 1E-398dd, 9.999999999999999E384dd};
constexpr _Decimal64 ce = 2.50dd;
static int st_int = (int)12.75dd;

static int kind(_Decimal32 x) { return 32; }
#define K(x) _Generic((x), _Decimal32: 32, _Decimal64: 64, _Decimal128: 128, double: 1, int: 2, default: 0)

int main(void) {
    volatile _Decimal64 a = 1.00dd, b = 2.0dd, z = 0;
    /* constants keep their quanta */
    dec_show(1.20dd);
/* expect: 1.20 */
    dec_show(120E-2df);
/* expect: 1.20 */
    dec_show(1.2E+3DL);
/* expect: 1.2E+3 */
    dec_show(0.000dd);
/* expect: 0.000 */
    dec_show(1234567.89df);
/* expect: 1234568 */
    dec_show(1'000.5dd);
/* expect: 1000.5 */
    dec_show(9.999999E96DF);
/* expect: 9.999999E+96 */
    dec_show(0.000001E-95df);
/* expect: 1E-101 */
    /* arithmetic: exact results with their preferred quanta */
    dec_show(a + b);
/* expect: 3.00 */
    dec_show(a * b);
/* expect: 2.000 */
    dec_show(a / b);
/* expect: 0.5 */
    dec_show(b - a);
/* expect: 1.00 */
    dec_show(a - a);
/* expect: 0.00 */
    dec_show(-a);
/* expect: -1.00 */
    dec_show(+a);
/* expect: 1.00 */
    dec_show(a / 3);
/* expect: 0.3333333333333333 */
    dec_show((_Decimal32)2 / 3);
/* expect: 0.6666667 */
    dec_show(1 / z);
/* expect: Infinity */
    dec_show(z / z);
/* expect: NaN */
    dec_show(-1 / z);
/* expect: -Infinity */
    /* folded the same */
    dec_show(1.00dd / 3);
/* expect: 0.3333333333333333 */
    dec_show(st_sum);
/* expect: 3.305 */
    dec_show(st_conv);
/* expect: 7 */
    dec_show(st_q);
/* expect: 0.250 */
    dec_show(st_tab[1]);
/* expect: -0.0 */
    dec_show(st_tab[2]);
/* expect: 1E-398 */
    dec_show(st_tab[3]);
/* expect: 9.999999999999999E+384 */
    dec_show(ce);
/* expect: 2.50 */
    print_i64(st_int);
/* expect: 12 */
    /* the usual arithmetic conversions: the wider decimal type */
    print_i64(K(1.0df + 1.0dd));
/* expect: 64 */
    print_i64(K(1.0df + 1));
/* expect: 32 */
    print_i64(K(1.0dl * 2.0df));
/* expect: 128 */
    print_i64(K(1 ? 1.0df : 2.0dl));
/* expect: 128 */
    print_i64(kind(5));
/* expect: 32 */
    print_i64(sizeof(_Decimal32) * 100 + sizeof(_Decimal64) * 10 + sizeof(_Decimal128) - 16);
/* expect: 480 */
    print_i64(_Alignof(_Decimal128));
/* expect: 16 */
    /* comparisons: cohorts are equal; NaN unordered */
    print_i64(1.0dd == 1.00dd);
/* expect: 1 */
    print_i64(a < b && b > a && a <= 1.0dd && a >= 1);
/* expect: 1 */
    print_i64((z / z) == (z / z));
/* expect: 0 */
    print_i64((z / z) != (z / z));
/* expect: 1 */
    /* truth: a zero of any exponent is false */
    print_i64(!0.000dd + !(a - a) + !!a + (z ? 10 : 0));
/* expect: 3 */
    /* conversions with integers */
    print_i64((long long)-7.99dd);
/* expect: -7 */
    print_u64((unsigned long long)1.8446744073709551615E19dl);
/* expect: 18446744073709551615 */
    dec_show((_Decimal64)-9223372036854775807LL);
/* expect: -9.223372036854776E+18 */
    dec_show((_Decimal128)-9223372036854775807LL);
/* expect: -9223372036854775807 */
    dec_show((_Decimal32)(unsigned char)200);
/* expect: 200 */
    {
        _BitInt(200) w = 123456789012345678901234567890123456789wb;
        dec_show((_Decimal128)w);
/* expect: 1.234567890123456789012345678901235E+38 */
        dec_show((_Decimal64)(unsigned _BitInt(100))1000);
/* expect: 1000 */
        _BitInt(130) v = (_BitInt(130))1.5E30dl;
        print_i64(v == 1500000000000000000000000000000wb);
/* expect: 1 */
        unsigned _BitInt(70) u = 123.9dd;
        print_u64((unsigned long long)u);
/* expect: 123 */
    }
    /* with the binary floating types: correctly rounded, the quantum the
       one nearest 0 that is exact */
    {
        volatile double h = 0.5, t = 0.1, hundred = 100.0;
        volatile float fl = 1.5f;
        dec_show((_Decimal64)h);
/* expect: 0.5 */
        dec_show((_Decimal64)t);
/* expect: 0.1000000000000000 */
        dec_show((_Decimal128)t);
/* expect: 0.1000000000000000055511151231257827 */
        dec_show((_Decimal32)hundred);
/* expect: 100 */
        dec_show((_Decimal32)fl);
/* expect: 1.5 */
        print_f64((double)0.1dd);
/* expect: 0.1 */
        print_f64((double)(float)1.25df);
/* expect: 1.25 */
        double d = 2.5dd; /* (an assignment converts) */
        print_f64(d);
/* expect: 2.5 */
        long double l = 1.5dl;
        print_f64((double)l);
/* expect: 1.5 */
        dec_show((_Decimal64)(long double)0.25);
/* expect: 0.25 */
        double _Complex zc = 3.5dd; /* (to a complex type: the real part) */
        print_f64(__builtin_creal(zc));
/* expect: 3.5 */
    }
    /* ++, --, compound assignment, also of integers by decimal values */
    {
        _Decimal32 x = 7.0df;
        x++;
        dec_show(x);
/* expect: 8.0 */
        --x;
        dec_show(x);
/* expect: 7.0 */
        x += 0.25df;
        dec_show(x);
/* expect: 7.25 */
        x *= 4;
        dec_show(x);
/* expect: 29.00 */
        int i = 10;
        i *= 1.5dd;
        print_i64(i);
/* expect: 15 */
        _Decimal128 y = 1;
        y /= 8;
        dec_show(y);
/* expect: 0.125 */
    }
    return 0;
}

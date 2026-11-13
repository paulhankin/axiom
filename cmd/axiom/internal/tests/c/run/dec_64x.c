/* _Decimal64x (C23 Annex H.2.4): a type of its own in decimal128's format
   (H.2.4p4): 16 bytes, aligned to 16; its constants (d64x, D64x) and
   Annex H.5.2's dN suffixes; the usual arithmetic conversions (H.4.3:
   _Decimal64 and _Decimal32 convert to it, it converts to _Decimal128, of
   the same values); conversions with _Decimal128 that change no bits (a
   signaling NaN stays one), folded and at run time; its arithmetic, as
   decimal128's; arguments, results and variadic arguments. */
/* flags: --c-std=c23 */
#include <stdarg.h>
#include "host.h"
#include "../dec_show.h"

#define K(x) _Generic((x), _Decimal32: 32, _Decimal64: 64, _Decimal64x: 65, _Decimal128: 128, default: 0)

static _Decimal64x st_a = 1.10d64x + 2.205D64x;    /* folded */
static _Decimal64x st_b = 1.000dl / 4;              /* a _Decimal128 value converted */
static _Decimal64x st_c = 7;
static _Decimal128 st_d = 1E-6176d64x;              /* the least subnormal of both */
static const _Decimal64x st_tab[] = {1.5d64x, -0.0d64x, 9.999999999999999999999999999999999E6144d64x};
constexpr _Decimal64x ce = 2.50d64x;

static _Decimal64x twice(_Decimal64x x) { return x + x; }
static _Decimal128 mix(_Decimal64x a, _Decimal32 b, _Decimal64x c, _Decimal64 d) { return a * b + c * d; }

static _Decimal64x sum(int n, ...) {
    va_list ap;
    _Decimal64x s = 0;
    va_start(ap, n);
    for (int i = 0; i < n; i++)
        s += va_arg(ap, _Decimal64x);
    va_end(ap);
    return s;
}

struct S { char c; _Decimal64x x; };

int main(void) {
    volatile _Decimal64x a = 1.1d64x, b = 3.D64x;
    volatile _Decimal128 dl = 2.5dl;
    volatile _Decimal64 dd = 0.1dd;
    volatile _Decimal32 df = 0.5df;
    print_i64(sizeof(_Decimal64x) * 100 + _Alignof(_Decimal64x));
/* expect: 1616 */
    print_i64(__builtin_offsetof(struct S, x));
/* expect: 16 */
    print_i64(K(a + dl) * 1000 + K(a + dd));
/* expect: 128065 */
    print_i64(K(df * a) * 1000 + K(a + 1) + K(1.0d32) + K(1.0d64) + K(1.0D128));
/* expect: 65289 */
    dec_show(st_a);
/* expect: 3.305 */
    dec_show(st_b);
/* expect: 0.250 */
    dec_show(st_c);
/* expect: 7 */
    dec_show(st_d);
/* expect: 1E-6176 */
    dec_show(st_tab[2]);
/* expect: 9.999999999999999999999999999999999E+6144 */
    dec_show(st_tab[1]);
/* expect: -0.0 */
    dec_show(ce * 2);
/* expect: 5.00 */
    dec_show(a * b);
/* expect: 3.3 */
    dec_show(a / 3);
/* expect: 0.3666666666666666666666666666666667 */
    dec_show(a - dl);
/* expect: -1.4 */
    dec_show((_Decimal64)(a / 3));
/* expect: 0.3666666666666667 */
    dec_show((_Decimal64x)dd + df);
/* expect: 0.6 */
    dec_show(twice(a));
/* expect: 2.2 */
    dec_show(mix(a, df, b, dd));
/* expect: 0.85 */
    dec_show(sum(3, a, (_Decimal64x)dl, b));
/* expect: 6.6 */
    /* the same bits both ways: a signaling NaN, a non-canonical coefficient */
    {
        unsigned long long w[2] = {0x1234, 0x7e00000000000000ULL};
        _Decimal128 sn;
        memcpy(&sn, w, 16);
        volatile _Decimal128 vs = sn;
        _Decimal64x x = vs;
        _Decimal128 back = x;
        unsigned long long r[2];
        memcpy(r, &back, 16);
        print_bool(r[0] == 0x1234 && r[1] == 0x7e00000000000000ULL);
/* expect: true */
    }
    {
        _Decimal64x x = 2.5d64x;
        x++;
        x *= 2;
        x -= dd;
        dec_show(x);
/* expect: 6.9 */
        print_bool(x > 6.8d64x && x < dl * 3 && x != 6.8dl && x == 6.90dd);
/* expect: true */
        print_i64((long long)(x * 100) + (int)(_Decimal64x)1E3dd);
/* expect: 1690 */
        print_f64((double)(a * b));
/* expect: 3.3 */
        dec_show((_Decimal64x)2.5);
/* expect: 2.5 */
        print_bool(!(_Decimal64x)0.000d64x && (x ? 1 : 0));
/* expect: true */
    }
    return 0;
}

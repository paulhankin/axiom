/* The decimal floating types with the interchange and extended binary ones
   (C23 6.3.1.5, Annex H): conversions both ways, folded and at run time,
   correctly rounded (_Float16 once, from the decimal value), and _Float128's and _Float32's own formats (not
   long double's or double's). */
/* flags: --c-std=c23 */
#include "host.h"
#include "../dec_show.h"

static const _Decimal64 k_from_h = (_Float16)0.1f16;  /* folded */
static const _Float16 k_to_h = 0.1dd;                 /* folded */
static const _Float32 k_to_f32 = 0.1dd;
static const _Float128 k_to_q = 0.1dd;

int main(void) {
    volatile _Float16 h = 0.1f16;
    volatile _Float32 f = 0.1f32;
    volatile _Float128 q = 0.1f128;
    volatile _Decimal64 d = 0.1dd;
    volatile _Decimal128 dl = 1.000000000000000000000000000000001dl;
    dec_show((_Decimal64)h);
/* expect: 0.0999755859375 */
    dec_show(k_from_h);
/* expect: 0.0999755859375 */
    print_f64((double)(_Float16)d);
/* expect: 0.0999755859375 */
    print_f64((double)k_to_h);
/* expect: 0.0999755859375 */
    print_bool((_Float32)d == 0.1f);
/* expect: true */
    print_bool(k_to_f32 == 0.1f);
/* expect: true */
    dec_show((_Decimal32)f);
/* expect: 0.1000000 */
    /* binary128 keeps what double cannot: 1 + 10^-33 is not 1 there */
    print_bool((_Float128)dl != (_Float128)1);
/* expect: true */
    print_bool(k_to_q == 0.1f128);
/* expect: true */
    print_bool((_Float128)d == q);
/* expect: true */
    dec_show((_Decimal128)q);
/* expect: 0.1000000000000000000000000000000000 */
    return 0;
}

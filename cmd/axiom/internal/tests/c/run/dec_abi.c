/* The decimal floating types across calls between units (cgen_abi.ax: in
   floating-point registers as the binary types of their sizes, as types of
   their own in homogeneous aggregates; Windows x64 as integers), structures
   of them by value, variadic arguments (not promoted), function pointers. */
/* flags: --c-std=c23 */
/* units: abi/dec_callee.c */
#include "host.h"
#include "../dec_show.h"

struct d2 { _Decimal64 a, b; };
struct d3f { _Decimal32 a, b, c; };
struct mix { _Decimal32 a; float b; };
struct dq { _Decimal128 q; };
struct big { _Decimal64 v[5]; int n; };
_Decimal32 add32(_Decimal32, _Decimal32);
_Decimal64 add64(_Decimal64, _Decimal64);
_Decimal128 add128(_Decimal128, _Decimal128);
_Decimal64 many(_Decimal64, int, _Decimal32, double, _Decimal128, _Decimal64, _Decimal64, _Decimal64, _Decimal64,
                _Decimal64, _Decimal64, _Decimal32);
struct d2 swap2(struct d2);
_Decimal32 sum3(struct d3f);
float mixf(struct mix);
struct dq twiceq(struct dq);
_Decimal64 bigsum(struct big);
_Decimal128 vsum(int, ...);

int main(void) {
    dec_show(add32(1.25df, 2.5df));
/* expect: 3.75 */
    dec_show(add64(1E300dd, 2.000dd));
/* expect: 1.000000000000000E+300 */
    dec_show(add128(1.000000000000000000000000000000001dl, 1));
/* expect: 2.000000000000000000000000000000001 */
    dec_show(many(1.0dd, 2, 3.0df, 4.0, 5.0dl, 6, 7, 8, 9, 10, 11, 12.0df));
/* expect: 78.0 */
    struct d2 s = swap2((struct d2){1.5dd, 2.25dd});
    dec_show(s.a);
/* expect: 2.25 */
    dec_show(s.b);
/* expect: 1.5 */
    dec_show(sum3((struct d3f){1, 2, 3.5df}));
/* expect: 6.5 */
    print_f64(mixf((struct mix){1.5df, 2.25f}));
/* expect: 3.75 */
    dec_show(twiceq((struct dq){1.5dl}).q);
/* expect: 3.0 */
    dec_show(bigsum((struct big){{1, 2, 3, 4, 5.5dd}, 5}));
/* expect: 15.5 */
    dec_show(vsum(5, 32, 1.5df, 64, 2.25dd, 128, 3.125dl, 0, 0.5, 32, 10.0df));
/* expect: 17.375 */
    _Decimal64 (*fp)(_Decimal64, _Decimal64) = add64;
    dec_show(fp(0.1dd, 0.2dd));
/* expect: 0.3 */
    return 0;
}

/* The decimal types' NaN payloads read from strtodN's and fscanf's
   n-char-sequences (C23 7.24.1.6): the sequence as an unsigned integer
   (decimal, 0x hexadecimal, 0 octal), the NaN's payload when canonical
   (below 10^(p - 1)), else 0; another sequence gives the payload 0. And
   exact elementary results with the quanta C23 prefers. */
/* flags: --c-std=c23 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    _Decimal64 a = strtod64("NAN(123)", 0), b = strtod64("-nan(0x1f)", 0), c = strtod64("nan(abc)", 0);
    _Decimal64 e = strtod64("nan(9999999999999999)", 0), o = strtod64("nan(017)", 0);
    _Decimal128 g = strtod128("nan(123456789012345678901234567890123)", 0);
    _Decimal32 h;
    sscanf("nan(42)", "%Ha", &h);
    printf("%Da %Da %Da %Da %Da %DDa %Ha\n", getpayloadd64(&a), getpayloadd64(&b), getpayloadd64(&c),
           getpayloadd64(&e), getpayloadd64(&o), getpayloadd128(&g), getpayloadd32(&h));
    printf("%d %d\n", signbit(b) != 0, isnan(a) && !issignaling(a));
    printf("%Da %Da %Da %Da %Da\n", atan2pid64(1.0dd, -1.0dd), atanpid64(-1.0dd), acospid64(0.0dd), tanpid64(0.25dd),
           exp2d64(-3.0dd));
    printf("%Da %Da %Da %Da\n", exp10d64(2.0dd), exp2m1d64(4.0dd), powd64(2.25dd, 0.5dd), compoundnd64(0.5dd, 3));
    return 0;
}
// expect: 123 31 0 0 15 123456789012345678901234567890123 42
// expect: 1 1
// expect: 0.75 -0.25 0.5 1 0.125
// expect: 100 15 1.5 3.375

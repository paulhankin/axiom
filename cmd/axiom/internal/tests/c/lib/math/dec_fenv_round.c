/* #pragma STDC FENV_DEC_ROUND (C23 7.6.3): the static decimal rounding
   direction of its scope, for decimal constants and the constant
   expressions of static initializers (at translation), for the operators,
   and for the functions of its table (sqrtd64, strtod64, printf); the other
   functions run in the dynamic direction, which the scope leaves as it
   was. Every direction, FE_DEC_TONEARESTFROMZERO too, on every target (the
   decimal arithmetic is software). */
/* flags: --c-std=c23 */
#include <fenv.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#pragma STDC FENV_DEC_ROUND FE_DEC_DOWNWARD
static _Decimal64 s_down = 2.0dd / 3, s_c = 1.00000000000000009dd;
#pragma STDC FENV_DEC_ROUND FE_DEC_UPWARD
static _Decimal64 s_up = 2.0dd / 3;
#pragma STDC FENV_DEC_ROUND FE_DEC_TONEARESTFROMZERO
static _Decimal32 s_a = 1.0000005df, s_b = 1.0000015df, s_n = -1.0000025df;
#pragma STDC FENV_DEC_ROUND FE_DEC_TOWARDZERO
static _Decimal32 s_z = -2.0df / 3;
#pragma STDC FENV_DEC_ROUND FE_DEC_DYNAMIC

static _Decimal64 third(_Decimal64 x)
{
    return x / 3; /* in the dynamic direction */
}

int main(void)
{
    volatile _Decimal64 one = 1, two = 2;
    printf("%Da %Da %Da\n", s_down, s_up, s_c);
    printf("%Ha %Ha %Ha %Ha\n", s_a, s_b, s_n, s_z);
    {
#pragma STDC FENV_DEC_ROUND FE_DEC_UPWARD
        _Decimal64 a = one / 3, b = third(one), c = sqrtd64(two), d = strtod64("0.33333333333333333", 0);
        printf("%Da %Da %Da %Da %d\n", a, b, c, d, fe_dec_getround() == FE_DEC_TONEAREST);
        printf("%.3De\n", one / 3);
    }
    {
#pragma STDC FENV_DEC_ROUND FE_DEC_DOWNWARD
        _Decimal64 a = one / 3, c = sqrtd64(two);
        printf("%Da %Da\n", a, c);
        if (one / 3 == 0.3333333333333333dd)
            puts("down in the condition");
        fe_dec_setround(FE_DEC_UPWARD); /* (the dynamic direction, for after the scope) */
        a = third(one);
        printf("%Da\n", a);
    }
    printf("%Da %d\n", one / 3, fe_dec_getround() == FE_DEC_UPWARD);
    fe_dec_setround(FE_DEC_TONEAREST);
    {
#pragma STDC FENV_DEC_ROUND FE_DEC_TONEARESTFROMZERO
        volatile _Decimal32 x = 1.000000df, y = 0.0000005df;
        _Decimal32 r = x + y, q = -x - y;
        printf("%Ha %Ha\n", r, q);
    }
    return 0;
}
// expect: 0.6666666666666666 0.6666666666666667 1.000000000000000
// expect: 1.000001 1.000002 -1.000003 -0.6666666
// expect: 0.3333333333333334 0.3333333333333333 1.414213562373096 0.3333333333333334 1
// expect: 3.334e-01
// expect: 0.3333333333333333 1.414213562373095
// expect: down in the condition
// expect: 0.3333333333333334
// expect: 0.3333333333333334 1
// expect: 1.000001 -1.000001

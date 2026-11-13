/* The decimal floating operations' exceptions in <fenv.h>'s flags (on the
   targets that have flags: the software arithmetic's merged with the
   hardware's), and the decimal rounding direction in the environment
   (fegetenv, fesetenv, feholdexcept, fegetmode, fesetmode, FE_DFL_ENV). */
/* flags: --c-std=c23 */
/* native-only */
#include <fenv.h>
#include <math.h>
#include <stdio.h>

#pragma STDC FENV_DEC_ROUND FE_DEC_DYNAMIC

/* Under FENV_ACCESS ON a decimal operation of constants is done at run
   time, in the dynamic decimal direction, raising its exceptions. */
static _Decimal64 third_up(int *inexact) {
#pragma STDC FENV_ACCESS ON
    fe_dec_setround(FE_DEC_UPWARD);
    feclearexcept(FE_ALL_EXCEPT);
    _Decimal64 r = 1.0dd / 3.0dd;
    *inexact = fetestexcept(FE_INEXACT) != 0;
    fe_dec_setround(FE_DEC_TONEAREST);
    return r;
}

static volatile _Decimal64 one = 1, three = 3, zero = 0, big = 9E384dd, tiny = 1E-398dd;

static void flags(void) {
    int f = fetestexcept(FE_ALL_EXCEPT);
    printf("%s%s%s%s%s|", f & FE_INVALID ? "i" : "", f & FE_DIVBYZERO ? "z" : "", f & FE_OVERFLOW ? "o" : "",
           f & FE_UNDERFLOW ? "u" : "", f & FE_INEXACT ? "x" : "");
    feclearexcept(FE_ALL_EXCEPT);
}

int main(void) {
    volatile _Decimal64 r;
    feclearexcept(FE_ALL_EXCEPT);
    r = one / three;
    flags();
    r = one / zero;
    flags();
    r = zero / zero;
    flags();
    r = big * 10;
    flags();
    r = tiny / 3;
    flags();
    r = one + three;
    flags();
    r = (zero / zero < one);
    flags();
    r = __builtin_axiom_dec(64, 0, 0x7e00000000000000) == one;
    flags();
    r = sqrtd64(-one);
    flags();
    r = rintd64(one / 2);
    flags();
    (void)r;
/* expect: x|z|i|ox|ux||i|i|i|x| */
    printf("\n");
    /* raised by decimal arithmetic, then tested, saved and cleared */
    {
        fenv_t e;
        fexcept_t fx;
        r = one / three;
        printf("%d ", fetestexcept(FE_INEXACT) != 0);
        fegetexceptflag(&fx, FE_INEXACT);
        feclearexcept(FE_INEXACT);
        printf("%d ", fetestexcept(FE_INEXACT) != 0);
        fesetexceptflag(&fx, FE_INEXACT);
        printf("%d ", fetestexcept(FE_INEXACT) != 0);
        fe_dec_setround(FE_DEC_UPWARD);
        fegetenv(&e);
        fe_dec_setround(FE_DEC_DOWNWARD);
        feholdexcept(&e);
        printf("%d %d ", fe_dec_getround(), fetestexcept(FE_ALL_EXCEPT));
        fesetenv(&e);
        printf("%d %d ", fe_dec_getround(), fetestexcept(FE_INEXACT) != 0);
        fesetenv(FE_DFL_ENV);
        printf("%d ", fe_dec_getround());
        femode_t m;
        fe_dec_setround(FE_DEC_TOWARDZERO);
        fegetmode(&m);
        fesetmode(FE_DFL_MODE);
        printf("%d ", fe_dec_getround());
        fesetmode(&m);
        printf("%d", fe_dec_getround());
        fe_dec_setround(FE_DEC_TONEAREST);
/* expect: 1 0 1 1 0 1 1 0 0 3 */
        printf("\n");
    }
    {
        /* iseqsig: equality, "invalid" for a quiet NaN too (== is quiet) */
        volatile _Decimal64 z = 0.0dd, nz = -0.0dd, q = DEC_NAN;
        volatile _Decimal128 w = 1.5dl;
        int a, b, c, d;
        feclearexcept(FE_ALL_EXCEPT);
        a = iseqsig(z, nz);
        b = fetestexcept(FE_INVALID) != 0;
        c = (q == z);
        d = fetestexcept(FE_INVALID) != 0;
        printf("%d %d %d %d ", a, b, c, d);
        a = iseqsig(q, z);
        b = fetestexcept(FE_INVALID) != 0;
        printf("%d %d %d\n", a, b, iseqsig(w, 1.50dl));
        feclearexcept(FE_ALL_EXCEPT);
    }
/* expect: 1 0 0 0 0 1 1 */
    {
        int ix = 0;
        _Decimal64 r = third_up(&ix);
        printf("%d %d\n", r == 0.3333333333333334dd, ix);
    }
/* expect: 1 1 */
    return 0;
}

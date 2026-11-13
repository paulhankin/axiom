/* #pragma STDC FENV_ACCESS ON (7.6.1, F.8): in its scope the optimizer
   leaves the floating operations where they are, as often as written, in
   the rounding mode of that moment, raising their exceptions (ir.Func's
   strict_fp, which the passes consult): each line below fails if one of
   them folds (opt.ax, combine.ax), merges (gvn.ax), moves out of a loop
   (gvn.ax's licm), removes (the dead code passes, the backends' own), sinks
   (x64.ax) or fuses a comparison past a call (x64.ax, a64.ax). An
   automatic object's constant initializer is evaluated at run time
   (F.8.5); the relational operators signal "invalid" for a NaN (F.9.3). */
/* flags: --c-std=c11 */
/* native-only */
#include "print.h"
#include <fenv.h>

#pragma STDC FENV_ACCESS ON

static double third(double a)
{
    return a / 3.0;
}

static double sum4(const double *v)
{
    return (v[0] + v[1]) + (v[2] + v[3]);
}

int main(void)
{
    double a = 1.0, b = 3.0, up, dn, x1, x2, s[4], t1, t2, nn;
    int modes[4] = {FE_UPWARD, FE_DOWNWARD, FE_UPWARD, FE_DOWNWARD};
    int i;
    volatile double z = 0.0, qn = 0.0;
    /* folding: a constant initializer of an automatic object, at run time */
    fesetround(FE_UPWARD);
    up = 1.0 / 3.0;
    fesetround(FE_DOWNWARD);
    dn = 1.0 / 3.0;
    fesetround(FE_TONEAREST);
    print_bool(up > dn);
    /* value numbering: the same expression in two modes */
    fesetround(FE_UPWARD);
    x1 = a / b;
    fesetround(FE_DOWNWARD);
    x2 = a / b;
    fesetround(FE_TONEAREST);
    print_bool(x1 > x2);
    /* dead code: an unused division still raises divide-by-zero */
    feclearexcept(FE_ALL_EXCEPT);
    {
        double unused = a / z;
        (void)unused;
    }
    print_bool(fetestexcept(FE_DIVBYZERO) != 0);
    /* an expression statement of constants: computed, inexact */
    feclearexcept(FE_ALL_EXCEPT);
    (void)(1.0 / 3.0);
    print_bool(fetestexcept(FE_INEXACT) != 0);
    /* loop invariants: computed in each iteration's mode */
    for (i = 0; i < 4; i++) {
        fesetround(modes[i]);
        s[i] = a / b;
    }
    fesetround(FE_TONEAREST);
    print_bool(s[0] > s[1] && s[2] > s[3] && s[0] == s[2]);
    /* an inlined callee: strict in the caller too */
    fesetround(FE_UPWARD);
    t1 = third(1.0);
    fesetround(FE_DOWNWARD);
    t2 = third(1.0);
    fesetround(FE_TONEAREST);
    print_bool(t1 > t2);
    /* sums of an array (the vectorizer's): in the mode */
    {
        double v[4] = {1.0, 0x1p-60, 1.0, 0x1p-60};
        double r1, r2;
        fesetround(FE_UPWARD);
        r1 = sum4(v);
        fesetround(FE_TONEAREST);
        r2 = sum4(v);
        print_bool(r1 > r2);
    }
    /* a comparison is made where it is: "invalid" before the test */
    nn = qn / qn;
    feclearexcept(FE_ALL_EXCEPT);
    i = nn < 1.0;
    print_bool(fetestexcept(FE_INVALID) != 0 && !i);
    /* the quiet comparison macros do not signal */
    feclearexcept(FE_ALL_EXCEPT);
    i = __builtin_axiom_isless(nn, 1.0);
    print_bool(fetestexcept(FE_INVALID) == 0 && !i);
    return 0;
}
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */

/* --c-fenv-access: every function as under #pragma STDC FENV_ACCESS ON. */
/* flags: --c-std=c11 --c-fenv-access */
/* native-only */
#include "print.h"
#include <fenv.h>

int main(void)
{
    double a = 1.0, b = 3.0, x1, x2;
    fesetround(FE_UPWARD);
    x1 = a / b;
    fesetround(FE_DOWNWARD);
    x2 = a / b;
    fesetround(FE_TONEAREST);
    print_bool(x1 > x2);
    feclearexcept(FE_ALL_EXCEPT);
    (void)(1.0 / 3.0);
    print_bool(fetestexcept(FE_INEXACT) != 0);
    return 0;
}
/* expect: true */
/* expect: true */

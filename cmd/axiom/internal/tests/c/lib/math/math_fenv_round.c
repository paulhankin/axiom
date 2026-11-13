/* #pragma STDC FENV_ROUND (C23 7.6.2): the static rounding direction of its
   scope, for the floating operators and the functions of 7.6.2p4's table
   (sqrt here); the other functions run in the dynamic mode, which the
   scope's end leaves as it was. */
/* flags: --c-std=c23 */
/* native-only */
#include "print.h"
#include <fenv.h>
#include <math.h>

static double tenth(double a)
{
    return a / 10.0; /* in the dynamic mode */
}

int main(void)
{
    volatile double one = 1.0;
    double u, d, n, c, s;
    {
#pragma STDC FENV_ROUND FE_UPWARD
        u = one / 10.0;
        c = tenth(one);
    }
    {
#pragma STDC FENV_ROUND FE_DOWNWARD
        d = one / 10.0;
    }
    n = one / 10.0;
    print_bool(u > d && (u == n || d == n));
    print_bool(c == n);
    print_bool(fegetround() == FE_TONEAREST);
    {
#pragma STDC FENV_ROUND FE_UPWARD
        s = sqrt(2.0 * one);
    }
    print_bool(s * s >= 2.0);
    {
#pragma STDC FENV_ROUND FE_TOWARDZERO
        u = -one / 10.0;
    }
    print_bool(u > -n);
    /* the dynamic mode, set in the scope by a call, stays after it */
    {
#pragma STDC FENV_ROUND FE_UPWARD
        fesetround(FE_DOWNWARD);
        u = one / 10.0;
    }
    print_bool(fegetround() == FE_DOWNWARD && u > d);
    fesetround(FE_TONEAREST);
    return 0;
}
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */
/* expect: true */

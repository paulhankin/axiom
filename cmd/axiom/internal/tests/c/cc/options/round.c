#include <fenv.h>
#include <stdio.h>
/* -frounding-math: the rounding direction set at run time is the one used */
static double third(double x) { return x / 3.0; }
int main(void) {
    fesetround(FE_UPWARD);
    double a = third(1.0);
    fesetround(FE_DOWNWARD);
    double b = third(1.0);
    fesetround(FE_TONEAREST);
    printf("%d\n", a != b);
    return 0;
}

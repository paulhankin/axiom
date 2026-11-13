/* The conversions of decimal floating values round in the decimal rounding
   direction (fe_dec_setround), as binary ones round in the binary one
   (C23 7.23.6.1p13, 7.6.3): e, f, g and a with a precision, and the
   halfway case away from zero that only the decimal direction has. */
/* flags: --c-std=c23 */
#include <fenv.h>
#include <stdio.h>

static void show(int mode, const char *name) {
    fe_dec_setround(mode);
    printf("%s: %.1Df %.1Df %.1Df %.1Df %.2De %.2Ha\n", name, 2.25dd, 2.35dd, -2.25dd, 2.21dd, 1.235dd, 1.25df);
}

int main(void) {
    show(FE_DEC_TONEAREST, "nearest");
    show(FE_DEC_TONEARESTFROMZERO, "from zero");
    show(FE_DEC_UPWARD, "upward");
    show(FE_DEC_DOWNWARD, "downward");
    show(FE_DEC_TOWARDZERO, "toward zero");
    fe_dec_setround(FE_DEC_TONEAREST);
    /* the binary direction does not move a decimal conversion */
    fesetround(FE_UPWARD);
    printf("%.1Df\n", 2.21dd);
    return 0;
}
// expect: nearest: 2.2 2.4 -2.2 2.2 1.24e+00 1.2
// expect: from zero: 2.3 2.4 -2.3 2.2 1.24e+00 1.3
// expect: upward: 2.3 2.4 -2.2 2.3 1.24e+00 1.3
// expect: downward: 2.2 2.3 -2.3 2.2 1.23e+00 1.2
// expect: toward zero: 2.2 2.3 -2.2 2.2 1.23e+00 1.2
// expect: 2.2

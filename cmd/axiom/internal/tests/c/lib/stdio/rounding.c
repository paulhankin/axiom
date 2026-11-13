/* The floating conversions honor the current rounding direction (F.5):
   fprintf's decimal and hexadecimal conversions, strtod, strtof and
   fscanf round to nearest (ties to even), toward zero, upward or
   downward as fesetround says, and an overflow toward zero gives the
   largest finite value (with ERANGE). WebAssembly has only to nearest. */
#include <stdio.h>
#include <stdlib.h>
#include <fenv.h>
#include <float.h>
#include <errno.h>

int main(void) {
    static const int modes[4] = {FE_TONEAREST, FE_TOWARDZERO, FE_UPWARD, FE_DOWNWARD};
    static const char *names[4] = {"near", "zero", "up", "down"};
    int m;
    for (m = 0; m < 4; m++) {
        double x;
        float y;
        char buf[64];
        fesetround(modes[m]);
        printf("%s: %.3f %.3f %.0f %.0f %.2e %.1a %.3g %.0f\n", names[m], 2.0 / 3, -2.0 / 3, 0.5, -1.5, 12345.0, 1.03125,
               0.0001234, 1e-300);
        printf("  %.2f %.2f %f %f %.1a\n", 0.0001, -0.0001, 2.5, 1e-7, -1.03125);
        x = strtod("0.1", NULL);
        y = strtof("0.1", NULL);
        printf("  strtod 0.1 %a %a %a\n", x, strtod("-0.1", NULL), (double)y);
        errno = 0;
        x = strtod("1e999", NULL);
        printf("  1e999 %g %d -1e999 %g 1e-999 %a -1e-999 %a\n", x, errno == ERANGE, strtod("-1e999", NULL),
               strtod("1e-999", NULL), strtod("-1e-999", NULL));
        printf("  %a %a\n", strtod("1.00000000000000011102230246251565404236316680908203125", NULL),
               strtod("0x1.000000000000081p0", NULL));
        sscanf("2.000000000000000000001 -2.000000000000000000001", "%lf %f", &x, &y);
        printf("  sscanf %a %a\n", x, (double)y);
        snprintf(buf, sizeof buf, "%.17g", DBL_MAX);
        printf("  DBL_MAX %s back %d\n", buf, strtod(buf, NULL) == DBL_MAX);
    }
    return 0;
}
// native-only

// expect: near: 0.667 -0.667 0 -2 1.23e+04 0x1.0p+0 0.000123 0
// expect:   0.00 -0.00 2.500000 0.000000 -0x1.0p+0
// expect:   strtod 0.1 0x1.999999999999ap-4 -0x1.999999999999ap-4 0x1.99999ap-4
// expect:   1e999 inf 1 -1e999 -inf 1e-999 0x0p+0 -1e-999 -0x0p+0
// expect:   0x1p+0 0x1.0000000000001p+0
// expect:   sscanf 0x1p+1 -0x1p+1
// expect:   DBL_MAX 1.7976931348623157e+308 back 1
// expect: zero: 0.666 -0.666 0 -1 1.23e+04 0x1.0p+0 0.000123 0
// expect:   0.00 -0.00 2.500000 0.000000 -0x1.0p+0
// expect:   strtod 0.1 0x1.9999999999999p-4 -0x1.9999999999999p-4 0x1.999998p-4
// expect:   1e999 1.79769e+308 1 -1e999 -1.79769e+308 1e-999 0x0p+0 -1e-999 -0x0p+0
// expect:   0x1p+0 0x1p+0
// expect:   sscanf 0x1p+1 -0x1p+1
// expect:   DBL_MAX 1.7976931348623157e+308 back 0
// expect: up: 0.667 -0.666 1 -1 1.24e+04 0x1.1p+0 0.000124 1
// expect:   0.01 -0.00 2.500000 0.000001 -0x1.0p+0
// expect:   strtod 0.1 0x1.999999999999ap-4 -0x1.9999999999999p-4 0x1.99999ap-4
// expect:   1e999 inf 1 -1e999 -1.79769e+308 1e-999 0x0.0000000000001p-1022 -1e-999 -0x0p+0
// expect:   0x1.0000000000001p+0 0x1.0000000000001p+0
// expect:   sscanf 0x1.0000000000001p+1 -0x1p+1
// expect:   DBL_MAX 1.7976931348623158e+308 back 0
// expect: down: 0.666 -0.667 0 -2 1.23e+04 0x1.0p+0 0.000123 0
// expect:   0.00 -0.01 2.500000 0.000000 -0x1.1p+0
// expect:   strtod 0.1 0x1.9999999999999p-4 -0x1.999999999999ap-4 0x1.999998p-4
// expect:   1e999 1.79769e+308 1 -1e999 -inf 1e-999 0x0p+0 -1e-999 -0x0.0000000000001p-1022
// expect:   0x1p+0 0x1p+0
// expect:   sscanf 0x1p+1 -0x1.000002p+1
// expect:   DBL_MAX 1.7976931348623157e+308 back 0

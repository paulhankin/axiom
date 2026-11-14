/* flags: --c-std=gnu17 */
/* Division of float complex values whose parts span the format's range: the
   method's quotients and products are formed in double, so that one that
   would be subnormal (or overflow) in float does not lose the result's
   precision: (2^100 + 0i) / (2^10 + 1.3125 2^-140 i) is 2^90 - 1.3125 2^-60 i
   to float's precision (in float, d/c would be the one subnormal 2^-149). */
#include "host.h"

static volatile float xr = 0x1p100f, xi = 0.0f, yr = 0x1p10f, yi = 0x1.5p-140f;

int main(void) {
    float _Complex a = xr + xi * 1.0if;
    float _Complex b = yr + yi * 1.0if;
    float _Complex c = a / b;
    print_f64(__real__ c * 0x1p-90);
    print_f64(__imag__ c * 0x1p60f);
    return 0;
}
/* expect: 1 */
/* expect: -1.3125 */

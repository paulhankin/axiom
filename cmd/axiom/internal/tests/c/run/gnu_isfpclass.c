/* flags: --c-std=gnu17 */
/* A GNU dialect's floating-point classification builtins:
   __builtin_isfpclass(x, mask) is 1 if x is in a class of mask (0x1 a
   signaling NaN, 0x2 a quiet one, 0x4 -inf, 0x8 a negative normal number,
   0x10 a negative subnormal one, 0x20 -0, 0x40 +0, 0x80 a positive
   subnormal, 0x100 a positive normal, 0x200 +inf); __builtin_issignaling,
   __builtin_issubnormal and __builtin_iszero are C23's macros; and
   __builtin_nans, nansf, nansl are signaling NaNs (__builtin_nan quiet
   ones), in float, double and long double. */
#include "host.h"

#define ALL(x) (__builtin_isfpclass(x, 0x1) | __builtin_isfpclass(x, 0x2) << 1 | \
    __builtin_isfpclass(x, 0x4) << 2 | __builtin_isfpclass(x, 0x8) << 3 | __builtin_isfpclass(x, 0x10) << 4 | \
    __builtin_isfpclass(x, 0x20) << 5 | __builtin_isfpclass(x, 0x40) << 6 | __builtin_isfpclass(x, 0x80) << 7 | \
    __builtin_isfpclass(x, 0x100) << 8 | __builtin_isfpclass(x, 0x200) << 9)

static volatile float vf[8];
static volatile double vd[8];
static volatile long double vl[8];

int main(void) {
    vf[0] = __builtin_nansf(""); vf[1] = __builtin_nanf(""); vf[2] = -__builtin_inff(); vf[3] = -1.5f;
    vf[4] = -1e-40f; vf[5] = -0.0f; vf[6] = 1e-40f; vf[7] = __builtin_inff();
    vd[0] = __builtin_nans(""); vd[1] = __builtin_nan(""); vd[2] = -__builtin_inf(); vd[3] = -1.5;
    vd[4] = -1e-310; vd[5] = 0.0; vd[6] = 1e-310; vd[7] = 2.0;
    vl[0] = __builtin_nansl(""); vl[1] = __builtin_nanl(""); vl[2] = -__builtin_infl(); vl[3] = -1.5L;
    vl[4] = -0.0L; vl[5] = 0.0L; vl[6] = 3.0L; vl[7] = __builtin_infl();
    long f = 0, d = 0, l = 0;
    for (int i = 0; i < 8; i++) {
        f = f * 3 + __builtin_ctz(ALL(vf[i]));
        d = d * 3 + __builtin_ctz(ALL(vd[i]));
        l = l * 3 + __builtin_ctz(ALL(vl[i]));
    }
    print_i64(f);
    print_i64(__builtin_isfpclass(vf[3], 0x8 | 0x100) + __builtin_isfpclass(1.0, 0x100) * 10);
    print_i64(d);
    print_i64(l);
    print_i64(__builtin_issignaling(vd[0]) + __builtin_issignaling(vd[1]) * 2 + __builtin_issubnormal(vf[6]) * 4 +
              __builtin_iszero(vl[4]) * 8 + __builtin_issignaling(vl[0]) * 16 + __builtin_isnan(vl[0]) * 32);
    return 0;
}
/* expect: 1641 */
/* expect: 11 */
/* expect: 1649 */
/* expect: 1680 */
/* expect: 61 */

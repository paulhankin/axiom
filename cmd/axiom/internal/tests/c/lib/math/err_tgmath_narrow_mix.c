/* A narrowing type-generic macro with an argument of the other radix than
   its prefix's has no function (C23 7.27p11, H.13p5: undefined; here an
   error). */
/* flags: --c-std=c23 */
/* expect-error: C3230 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <tgmath.h>

int main(void) {
    return (int)d32add(1.0dd, 2.0);
}

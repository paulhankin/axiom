/* flags: --c-std=gnu17 */
/* __complex__ and __complex are a GNU dialect's spellings of _Complex. */
#include "host.h"

static double __complex__ twice(double __complex__ z) { return z + z; }

int main(void) {
    __complex float f = 1.0f + 2.0fi;
    volatile double __complex__ acc = 0;
    acc += twice(f);
    print_i64((long)__real__ acc * 10 + (long)__imag__ acc);
    return 0;
}
/* expect: 24 */

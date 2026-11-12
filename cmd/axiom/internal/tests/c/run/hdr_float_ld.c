/* flags: --c-std=c23 */
/* <float.h>'s long double macros in the format the target's C ABI gives it
   (__AXIOM_LDBL_WIDTH__: 80, the x87's; 128, binary128; 64, double's),
   each against its definition, written exactly in hexadecimal; and
   DECIMAL_DIG digits read back exactly. */
#include <float.h>
#include "host.h"

#if __AXIOM_LDBL_WIDTH__ == 80
#define P 64
#define MAX 0x1.fffffffffffffffep16383L
#elif __AXIOM_LDBL_WIDTH__ == 128
#define P 113
#define MAX 0x1.ffffffffffffffffffffffffffffp16383L
#else
#define P 53
#define MAX 0x1.fffffffffffffp1023L
#endif

int main(void) {
    volatile long double one = 1.0L;
    print_i64(LDBL_MANT_DIG == P && sizeof(long double) == (P == 53 ? 8 : 16));
    if (P == 53) {
        print_i64(LDBL_DIG == 15 && LDBL_MIN_EXP == -1021 && LDBL_MAX_EXP == 1024 && LDBL_MIN_10_EXP == -307 &&
                  LDBL_MAX_10_EXP == 308 && LDBL_DECIMAL_DIG == 17 && DECIMAL_DIG == 17);
        print_i64(LDBL_MIN == 0x1p-1022L && LDBL_TRUE_MIN == 0x1p-1074L);
    } else {
        print_i64(LDBL_DIG == (P == 64 ? 18 : 33) && LDBL_MIN_EXP == -16381 && LDBL_MAX_EXP == 16384 &&
                  LDBL_MIN_10_EXP == -4931 && LDBL_MAX_10_EXP == 4932 && LDBL_DECIMAL_DIG == (P == 64 ? 21 : 36) &&
                  DECIMAL_DIG == LDBL_DECIMAL_DIG);
        print_i64(LDBL_MIN == 0x1p-16382L && LDBL_TRUE_MIN == (P == 64 ? 0x1p-16445L : 0x1p-16494L));
    }
    print_i64(LDBL_MAX == MAX && LDBL_NORM_MAX == MAX && LDBL_EPSILON == (P == 64 ? 0x1p-63L : P == 113 ? 0x1p-112L : 0x1p-52L));
    print_i64(one + LDBL_EPSILON != one && one + LDBL_EPSILON / 2 == one && (one + LDBL_EPSILON) - one == LDBL_EPSILON);
    print_i64(LDBL_TRUE_MIN / 2 == 0 && LDBL_TRUE_MIN > 0 && LDBL_MIN - LDBL_TRUE_MIN < LDBL_MIN && LDBL_MAX * 2 > LDBL_MAX);
    print_i64(LDBL_IS_IEC_60559 == 1 && LDBL_HAS_SUBNORM == 1 && (long double)FLT_MAX < LDBL_MAX);
    return 0;
}

/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */

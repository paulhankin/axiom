/* flags: --c-std=c23 */
/* <float.h>: binary32 and binary64 (long double is double), with C23's
   additions (*_NORM_MAX, *_IS_IEC_60559, INFINITY, NAN). */
#include <float.h>
#include "host.h"

int main(void) {
    volatile float f = 1.0f;
    volatile double d = 1.0;
    print_i64(FLT_RADIX == 2 && FLT_MANT_DIG == 24 && DBL_MANT_DIG == 53 && LDBL_MANT_DIG == DBL_MANT_DIG);
    print_i64(FLT_DIG == 6 && DBL_DIG == 15 && FLT_DECIMAL_DIG == 9 && DBL_DECIMAL_DIG == 17 && DECIMAL_DIG == 17);
    print_i64(FLT_MIN_EXP == -125 && FLT_MAX_EXP == 128 && DBL_MIN_EXP == -1021 && DBL_MAX_EXP == 1024);
    print_i64(FLT_MIN_10_EXP == -37 && FLT_MAX_10_EXP == 38 && DBL_MIN_10_EXP == -307 && DBL_MAX_10_EXP == 308);
    print_i64(FLT_MAX == 0x1.fffffep127f && FLT_MIN == 0x1p-126f && FLT_EPSILON == 0x1p-23f && FLT_TRUE_MIN == 0x1p-149f);
    print_i64(DBL_MAX == 0x1.fffffffffffffp1023 && DBL_MIN == 0x1p-1022 && DBL_EPSILON == 0x1p-52 && DBL_TRUE_MIN == 0x1p-1074);
    print_i64(LDBL_MAX == DBL_MAX && LDBL_MIN == DBL_MIN && LDBL_EPSILON == DBL_EPSILON && LDBL_TRUE_MIN == DBL_TRUE_MIN);
    print_i64(FLT_NORM_MAX == FLT_MAX && DBL_NORM_MAX == DBL_MAX && LDBL_NORM_MAX == LDBL_MAX);
    print_i64(FLT_IS_IEC_60559 && DBL_IS_IEC_60559 && LDBL_IS_IEC_60559 && FLT_HAS_SUBNORM == 1 && FLT_EVAL_METHOD == 0);
    print_i64(f + FLT_EPSILON != f && f + FLT_EPSILON / 2 == f && d + DBL_EPSILON != d && d + DBL_EPSILON / 2 == d);
    print_i64(INFINITY > FLT_MAX && NAN != NAN && sizeof(INFINITY) == sizeof(float) && FLT_ROUNDS == 1);
    return 0;
}

/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */
/* expect: 1 */

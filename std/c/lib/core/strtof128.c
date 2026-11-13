/* strtof128 (<stdlib.h>, ISO/IEC TS 18661-3 12.4): the subject sequence of
   strtod, its value correctly rounded to _Float128 (binary128), by the
   formatted input's scanner (std/c/lib/stdio/scanfloat.c); HUGE_VAL_F128
   with the sign, or a value no greater than FLT128_MIN, and errno ERANGE
   out of range. */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <errno.h>
#include <stdlib.h>
#include "axc/core_impl.h"

_Float128 strtof128(const char *restrict nptr, char **restrict endptr) {
    int range;
    union {
        _Float128 v;
        unsigned char b[16];
    } u;
    __axc_strtobits(nptr, endptr, &range, 1, u.b);
    if (range)
        errno = ERANGE;
    return u.v;
}

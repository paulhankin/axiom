/* strtof16 (<stdlib.h>, ISO/IEC TS 18661-3 12.4): the subject sequence of
   strtod, its value correctly rounded to _Float16 (binary16), by the
   formatted input's scanner (std/c/lib/stdio/scanfloat.c); HUGE_VAL_F16
   with the sign, or a value no greater than FLT16_MIN, and errno ERANGE
   out of range. */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <errno.h>
#include <stdlib.h>
#include "axc/core_impl.h"

_Float16 strtof16(const char *restrict nptr, char **restrict endptr) {
    int range;
    union {
        _Float16 v;
        unsigned char b[2];
    } u;
    __axc_strtobits(nptr, endptr, &range, 0, u.b);
    if (range)
        errno = ERANGE;
    return u.v;
}

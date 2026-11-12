/* strtold (<stdlib.h>, C23 7.24.1.5): the subject sequence's value in
   long double's format (binary64, the x87 80-bit format or binary128, as
   LDBL_MANT_DIG says), correctly rounded, decimal and hexadecimal: the
   scanner of the formatted input's (std/c/lib/stdio/scanfloat.c); HUGE_VALL
   with the sign, or a value no greater than LDBL_MIN, and errno ERANGE out
   of range. */
#include <errno.h>
#include <stdlib.h>
#include "axc/core_impl.h"

long double strtold(const char *restrict nptr, char **restrict endptr) {
    int range;
    long double v = __axc_strtold(nptr, endptr, &range);
    if (range)
        errno = ERANGE;
    return v;
}

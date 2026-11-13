/* strtod128 (<stdlib.h>, C23 7.24.1.6): the subject sequence's value as a
   _Decimal128, correctly rounded in the decimal rounding direction, with
   the quantum exponent it is written with (when exact); errno ERANGE when
   it overflows or underflows (the scanner of the formatted input's:
   std/c/lib/stdio/scanfloat.c, __axc_strtodec). */
#include <errno.h>
#include <stdlib.h>

void __axc_strtodec(const char *s, char **end, int dfmt, void *dst, int *range);

__axiom_decimal128 strtod128(const char *restrict nptr, char **restrict endptr) {
    __axiom_decimal128 v;
    int range;
    __axc_strtodec(nptr, endptr, 3, &v, &range);
    if (range)
        errno = ERANGE;
    return v;
}

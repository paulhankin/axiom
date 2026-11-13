/* strfromf128 (<stdlib.h>, ISO/IEC TS 18661-3 12.3): fp formatted as
   snprintf(s, n, format, fp) would format a value of its format, for a
   format "%[.precision]conversion" with a conversion of a, A, e, E, f, F,
   g or G (std/c/lib/stdio/strfrombits.c). */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <stdlib.h>
#include "axc/core_impl.h"

int strfromf128(char *restrict s, size_t n, const char *restrict format, _Float128 fp) {
    union {
        _Float128 v;
        unsigned char b[16];
    } u;
    u.v = fp;
    return __axc_strfrombits(s, n, format, u.b, 1);
}

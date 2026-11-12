/* strtof (<stdlib.h>, C23 7.24.1.5): as strtod, rounded once to float. */
#include <errno.h>
#include <stdlib.h>
#include "axc/core_impl.h"

float strtof(const char *restrict nptr, char **restrict endptr) {
    int range;
    unsigned long long b = __axc_strtofp(nptr, endptr, 32, &range);
    float f;
    unsigned char *fp = (unsigned char *)&f;
    int k;
    for (k = 0; k < 4; k++)
        fp[k] = (unsigned char)(b >> (8 * k));
    if (range)
        errno = ERANGE;
    return f;
}

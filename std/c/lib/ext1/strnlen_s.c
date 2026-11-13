/* strnlen_s (C11 K.3.7.4.4): the length of the string s, or maxsize if no
   null character is in its first maxsize characters; 0 for a null
   pointer. No runtime-constraints. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>

size_t strnlen_s(const char *s, size_t maxsize) {
    const char *z;
    if (s == NULL)
        return 0;
    z = memchr(s, 0, maxsize);
    return z != NULL ? (size_t)(z - s) : maxsize;
}

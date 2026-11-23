/* strerrorlen_s (C11 K.3.7.4.3): the length of strerror's message for
   errnum (without its null character). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>

size_t strerrorlen_s(errno_t errnum) {
    return strlen(strerror(errnum));
}

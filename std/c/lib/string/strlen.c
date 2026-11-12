/* strlen (C23 7.26.6.4): the number of bytes before the terminating null
   byte. Words at a time once aligned (an aligned word never crosses into
   another page, so reading past the end of the string is harmless). */
#include <string.h>
#include "word.h"

size_t strlen(const char *s) {
    const char *p = s;
    while (!__AXIOM_ALIGNED(p)) {
        if (*p == 0) {
            return (size_t)(p - s);
        }
        p++;
    }
    const __axiom_word *w = (const __axiom_word *)p;
    while (!__AXIOM_HAS_ZERO(*w)) {
        w++;
    }
    p = (const char *)w;
    while (*p) {
        p++;
    }
    return (size_t)(p - s);
}

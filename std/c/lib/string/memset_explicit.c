/* memset_explicit (C23 7.26.6.2): as memset, for memory that is not used
   afterwards (sensitive data): the stores are volatile, so that they are
   done even then. */
#include <string.h>

void *memset_explicit(void *s, int c, size_t n) {
    volatile unsigned char *d = s;
    while (n--) {
        *d++ = (unsigned char)c;
    }
    return s;
}

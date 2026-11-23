/* memccpy (C23 7.26.2.2): copies bytes from s2 to s1 up to and including
   the first that is (unsigned char)c, at most n; returns the address after
   that byte's copy, or a null pointer if it was not found. */
#include <string.h>

void *memccpy(void *restrict s1, const void *restrict s2, int c, size_t n) {
    unsigned char *d = s1;
    const unsigned char *s = s2;
    unsigned char b = (unsigned char)c;
    while (n--) {
        if ((*d++ = *s++) == b) {
            return d;
        }
    }
    return NULL;
}

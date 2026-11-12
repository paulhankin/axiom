/* strxfrm (<string.h>, C23 7.26.4.5): s2 transformed so that strcmp of two
   results orders them as strcoll orders the strings: in the library's
   locales, the string itself. Writes at most n bytes (the terminating null
   included) to s1 if it fits, and returns the length of the transformed
   string (not counting the null); s1 may be null when n is 0. */
#include "axc/string_hosted.h"

size_t strxfrm(char *restrict s1, const char *restrict s2, size_t n) {
    size_t len = 0;
    while (s2[len])
        len++;
    if (len < n) {
        size_t i;
        for (i = 0; i <= len; i++)
            s1[i] = s2[i];
    }
    return len;
}

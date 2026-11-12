/* strcoll (<string.h>, C23 7.26.4.3): compares s1 and s2 as the LC_COLLATE
   category of the locale orders them: in each of the library's locales the
   order of their bytes as unsigned char (the "C" locale's, and the order of
   code points in UTF-8), as strcmp. */
#include "axc/string_hosted.h"

int strcoll(const char *s1, const char *s2) {
    const unsigned char *a = (const unsigned char *)s1, *b = (const unsigned char *)s2;
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (*a > *b) - (*a < *b);
}

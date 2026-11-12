/* strchr (C23 7.26.5.3): the first byte of the string s that is (char)c
   (the null byte included), or a null pointer. */
#include <string.h>

#undef strchr

char *strchr(const char *s, int c) {
    char ch = (char)c;
    for (;; s++) {
        if (*s == ch) {
            return (char *)s;
        }
        if (*s == 0) {
            return NULL;
        }
    }
}

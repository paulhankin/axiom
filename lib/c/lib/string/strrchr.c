/* strrchr (C23 7.26.5.6): the last byte of the string s that is (char)c
   (the null byte included), or a null pointer. */
#include <string.h>

#undef strrchr

char *strrchr(const char *s, int c) {
    char ch = (char)c;
    const char *r = NULL;
    for (;; s++) {
        if (*s == ch) {
            r = s;
        }
        if (*s == 0) {
            return (char *)r;
        }
    }
}

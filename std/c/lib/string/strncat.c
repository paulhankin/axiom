/* strncat (C23 7.26.3.2): appends at most n bytes of the string s2 to the
   string s1, then a null byte. */
#include <string.h>

char *strncat(char *restrict s1, const char *restrict s2, size_t n) {
    char *d = s1 + strlen(s1);
    for (; n > 0 && *s2; n--) {
        *d++ = *s2++;
    }
    *d = 0;
    return s1;
}

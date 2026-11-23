/* strncpy (C23 7.26.2.5): copies at most n bytes of the string s2 to s1,
   then null bytes until n are written. */
#include <string.h>

char *strncpy(char *restrict s1, const char *restrict s2, size_t n) {
    char *d = s1;
    for (; n > 0 && *s2; n--) {
        *d++ = *s2++;
    }
    for (; n > 0; n--) {
        *d++ = 0;
    }
    return s1;
}

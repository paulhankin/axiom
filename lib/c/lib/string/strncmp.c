/* strncmp (C23 7.26.4.4): compares at most n bytes of two strings, as
   unsigned char. */
#include <string.h>

int strncmp(const char *s1, const char *s2, size_t n) {
    const unsigned char *a = (const unsigned char *)s1;
    const unsigned char *b = (const unsigned char *)s2;
    for (; n > 0; n--, a++, b++) {
        if (*a != *b || *a == 0) {
            return *a - *b;
        }
    }
    return 0;
}

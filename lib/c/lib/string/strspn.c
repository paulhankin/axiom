/* strspn (C23 7.26.5.7): the length of the longest prefix of s1 of bytes
   in s2. */
#include <string.h>

size_t strspn(const char *s1, const char *s2) {
    unsigned char set[256] = { 0 };
    for (const unsigned char *q = (const unsigned char *)s2; *q; q++) {
        set[*q] = 1;
    }
    const unsigned char *p = (const unsigned char *)s1;
    while (set[*p]) {
        p++;
    }
    return (size_t)(p - (const unsigned char *)s1);
}

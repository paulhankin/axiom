/* strpbrk (C23 7.26.5.5): the first byte of s1 that is in s2, or a null
   pointer. */
#include <string.h>

#undef strpbrk

char *strpbrk(const char *s1, const char *s2) {
    s1 += strcspn(s1, s2);
    return *s1 ? (char *)s1 : NULL;
}

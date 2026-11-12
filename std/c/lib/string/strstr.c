/* strstr (C23 7.26.5.8): the first occurrence of the string s2 in the
   string s1 (s1 itself if s2 is empty), or a null pointer. */
#include <string.h>

#undef strstr

char *strstr(const char *s1, const char *s2) {
    size_t n = strlen(s2);
    if (n == 0) {
        return (char *)s1;
    }
    for (; (s1 = strchr(s1, *s2)) != NULL; s1++) {
        if (strncmp(s1, s2, n) == 0) {
            return (char *)s1;
        }
    }
    return NULL;
}

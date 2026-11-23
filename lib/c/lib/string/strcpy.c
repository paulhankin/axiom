/* strcpy (C23 7.26.2.4): copies the string s2, its null byte included, to
   s1. */
#include <string.h>

char *strcpy(char *restrict s1, const char *restrict s2) {
    char *d = s1;
    while ((*d++ = *s2++) != 0) {
    }
    return s1;
}

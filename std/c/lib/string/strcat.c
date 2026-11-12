/* strcat (C23 7.26.3.1): appends the string s2 to the string s1. */
#include <string.h>

char *strcat(char *restrict s1, const char *restrict s2) {
    char *d = s1 + strlen(s1);
    while ((*d++ = *s2++) != 0) {
    }
    return s1;
}

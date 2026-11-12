/* wmemmove (C17 7.29.4.2.4): the objects may overlap. */
#include <wchar.h>

wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n) {
    size_t i;
    /* (the addresses as integers, size_t being as wide as a pointer: the
       objects need not be in one array) */
    if ((size_t)s1 < (size_t)s2) {
        for (i = 0; i < n; i++)
            s1[i] = s2[i];
    } else {
        for (i = n; i > 0; i--)
            s1[i - 1] = s2[i - 1];
    }
    return s1;
}

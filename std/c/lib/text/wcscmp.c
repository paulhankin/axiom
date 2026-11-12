/* wcscmp (C17 7.29.4.4.1): wide characters compare as wchar_t values. */
#include <wchar.h>

int wcscmp(const wchar_t *s1, const wchar_t *s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 < *s2 ? -1 : *s1 > *s2;
}

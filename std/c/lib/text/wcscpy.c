/* wcscpy (C17 7.29.4.2.1). */
#include <wchar.h>

wchar_t *wcscpy(wchar_t *restrict s1, const wchar_t *restrict s2) {
    wchar_t *d = s1;
    while ((*d++ = *s2++) != 0) {
    }
    return s1;
}

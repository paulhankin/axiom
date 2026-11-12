/* wmemcpy (C17 7.29.4.2.3). */
#include <wchar.h>

wchar_t *wmemcpy(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n) {
    size_t i;
    for (i = 0; i < n; i++)
        s1[i] = s2[i];
    return s1;
}

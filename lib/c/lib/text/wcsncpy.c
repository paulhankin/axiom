/* wcsncpy (C17 7.29.4.2.2): at most n wide characters, the rest of the n
   null if s2 is shorter. */
#include <wchar.h>

wchar_t *wcsncpy(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n) {
    size_t i = 0;
    for (; i < n && s2[i]; i++)
        s1[i] = s2[i];
    for (; i < n; i++)
        s1[i] = 0;
    return s1;
}

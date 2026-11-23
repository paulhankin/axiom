/* wcsncat (C17 7.29.4.3.2): at most n wide characters of s2, then a null
   one. */
#include <wchar.h>

wchar_t *wcsncat(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n) {
    wchar_t *d = s1;
    while (*d)
        d++;
    while (n-- > 0 && *s2)
        *d++ = *s2++;
    *d = 0;
    return s1;
}

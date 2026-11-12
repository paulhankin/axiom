/* wcsxfrm (C17 7.29.4.4.4): with "C" collation (wcscoll is wcscmp) the
   transformed string is the string itself. Its length is returned; it is
   copied only if it fits in n wide characters with its null one. */
#include <wchar.h>

size_t wcsxfrm(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n) {
    size_t len = wcslen(s2);
    if (len < n)
        wmemcpy(s1, s2, len + 1);
    return len;
}

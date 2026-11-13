/* wcsxfrm (C17 7.29.4.4.4): a string whose wcscmp order is wcscoll's:
   the string itself where wchar_t is UTF-32, and where it is UTF-16 each
   unit's key (__axc_wcs_key: code point order). Its length is returned;
   it is written only if it fits in n wide characters with its null one. */
#include <axc/text.h>
#include <wchar.h>

size_t wcsxfrm(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n) {
    size_t len = wcslen(s2);
    if (len < n) {
#if __AXIOM_WCHAR_WIDTH__ == 16
        size_t i;
        for (i = 0; i <= len; i++)
            s1[i] = (wchar_t)__axc_wcs_key(s2[i]);
#else
        wmemcpy(s1, s2, len + 1);
#endif
    }
    return len;
}

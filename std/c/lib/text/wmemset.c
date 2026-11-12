/* wmemset (C17 7.29.4.6.2). */
#include <wchar.h>

wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n) {
    size_t i;
    for (i = 0; i < n; i++)
        s[i] = c;
    return s;
}

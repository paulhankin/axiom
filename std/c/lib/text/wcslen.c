/* wcslen (C17 7.29.4.6.1). */
#include <wchar.h>

size_t wcslen(const wchar_t *s) {
    const wchar_t *p = s;
    while (*p)
        p++;
    return (size_t)(p - s);
}

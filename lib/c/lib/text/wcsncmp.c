/* wcsncmp (C17 7.29.4.4.3). */
#include <wchar.h>

int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n) {
    for (; n > 0; n--, s1++, s2++) {
        if (*s1 != *s2)
            return *s1 < *s2 ? -1 : 1;
        if (*s1 == 0)
            break;
    }
    return 0;
}

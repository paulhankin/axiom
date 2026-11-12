/* wmemcmp (C17 7.29.4.4.5). */
#include <wchar.h>

int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n) {
    size_t i;
    for (i = 0; i < n; i++)
        if (s1[i] != s2[i])
            return s1[i] < s2[i] ? -1 : 1;
    return 0;
}

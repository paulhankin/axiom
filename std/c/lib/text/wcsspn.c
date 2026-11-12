/* wcsspn (C17 7.29.4.5.5). */
#include <wchar.h>

size_t wcsspn(const wchar_t *s1, const wchar_t *s2) {
    size_t i;
    for (i = 0; s1[i]; i++) {
        const wchar_t *p = s2;
        while (*p && *p != s1[i])
            p++;
        if (*p == 0)
            break;
    }
    return i;
}

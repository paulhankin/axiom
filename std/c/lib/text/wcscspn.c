/* wcscspn (C17 7.29.4.5.2). */
#include <wchar.h>

size_t wcscspn(const wchar_t *s1, const wchar_t *s2) {
    size_t i;
    for (i = 0; s1[i]; i++) {
        const wchar_t *p;
        for (p = s2; *p; p++)
            if (*p == s1[i])
                return i;
    }
    return i;
}

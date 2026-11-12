/* wcstok (C17 7.29.4.5.7): the position to go on from is kept in *ptr. */
#include <wchar.h>

wchar_t *wcstok(wchar_t *restrict s1, const wchar_t *restrict s2, wchar_t **restrict ptr) {
    wchar_t *s = s1 ? s1 : *ptr;
    wchar_t *end;
    if (!s)
        return 0;
    s += wcsspn(s, s2);
    if (*s == 0) {
        *ptr = s;
        return 0;
    }
    end = s + wcscspn(s, s2);
    if (*end) {
        *end = 0;
        *ptr = end + 1;
    } else {
        *ptr = end;
    }
    return s;
}

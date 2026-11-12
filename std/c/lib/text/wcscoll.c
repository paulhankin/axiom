/* wcscoll (C17 7.29.4.4.2): every locale collates as "C" does, by the
   values of the wide characters. */
#include <wchar.h>

int wcscoll(const wchar_t *s1, const wchar_t *s2) {
    return wcscmp(s1, s2);
}

/* wmemmove (C17 7.29.4.2.4): n wide characters, as memmove moves them. */
#include <axc/text.h>
#include <wchar.h>
#include <string.h>

wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n) {
    memmove(s1, s2, n * sizeof(wchar_t));
    return s1;
}

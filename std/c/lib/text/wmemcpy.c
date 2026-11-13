/* wmemcpy (C17 7.29.4.2.3): n wide characters, as memcpy copies them. */
#include <axc/text.h>
#include <wchar.h>
#include <string.h>

wchar_t *wmemcpy(wchar_t *restrict s1, const wchar_t *restrict s2, size_t n) {
    memcpy(s1, s2, n * sizeof(wchar_t));
    return s1;
}

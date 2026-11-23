/* wcsnlen_s (C11 K.3.9.2.4.1): the length of the wide string s, or
   maxsize if no null wide character is in its first maxsize; 0 for a null
   pointer. No runtime-constraints. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <wchar.h>

size_t wcsnlen_s(const wchar_t *s, size_t maxsize) {
    size_t n = 0;
    if (s == NULL)
        return 0;
    while (n < maxsize && s[n] != 0)
        n++;
    return n;
}

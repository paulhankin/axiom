/* mbstowcs (C17 7.22.8.1): mbsrtowcs from the initial state; with pwcs a
   null pointer, the length of the conversion (as POSIX has it). */
#include <axc/text.h>
#include <wchar.h>

size_t mbstowcs(wchar_t *restrict pwcs, const char *restrict s, size_t n) {
    mbstate_t st = {0};
    const char *p = s;
    return mbsrtowcs(pwcs, &p, n, &st);
}

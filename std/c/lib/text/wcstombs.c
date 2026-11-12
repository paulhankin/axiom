/* wcstombs (C17 7.22.8.2): wcsrtombs from the initial state; with s a null
   pointer, the length of the conversion (as POSIX has it). */
#include <axc/text.h>
#include <wchar.h>

size_t wcstombs(char *restrict s, const wchar_t *restrict pwcs, size_t n) {
    mbstate_t st = {0};
    const wchar_t *p = pwcs;
    return wcsrtombs(s, &p, n, &st);
}

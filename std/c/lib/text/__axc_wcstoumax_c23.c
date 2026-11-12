/* __axc_wcstoumax_c23: wcstoumax in C23 mode, whose header maps the name here: the
   0b and 0B prefixes too, for base 0 and 2 (C23 7.24.1.7). */
#include <inttypes.h>
#include <wchar.h>

uintmax_t __axc_wcstoumax_c23(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base) {
    return (uintmax_t)__axc_wcstoull_c23(nptr, endptr, base);
}

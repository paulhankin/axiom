/* __axc_wcstoimax_c23: wcstoimax in C23 mode, whose header maps the name here: the
   0b and 0B prefixes too, for base 0 and 2 (C23 7.24.1.7). */
#include <inttypes.h>
#include <wchar.h>

intmax_t __axc_wcstoimax_c23(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base) {
    return (intmax_t)__axc_wcstoll_c23(nptr, endptr, base);
}

/* wcstoimax (C17 7.8.2.4): wcstoll, intmax_t being a 64-bit type as long
   long is. */
#include <inttypes.h>
#include <wchar.h>

intmax_t wcstoimax(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base) {
    return (intmax_t)wcstoll(nptr, endptr, base);
}

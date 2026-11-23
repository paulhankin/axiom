/* wcstoumax (C17 7.8.2.4): wcstoull, uintmax_t being a 64-bit type as
   unsigned long long is. */
#include <inttypes.h>
#include <wchar.h>

uintmax_t wcstoumax(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base) {
    return (uintmax_t)wcstoull(nptr, endptr, base);
}

/* strtoumax (C17 7.8.2.3): strtoull, uintmax_t being a 64-bit type as
   unsigned long long is. */
#include <inttypes.h>
#include <axc/text.h>

uintmax_t strtoumax(const char *restrict nptr, char **restrict endptr, int base) {
    return (uintmax_t)strtoull(nptr, endptr, base);
}

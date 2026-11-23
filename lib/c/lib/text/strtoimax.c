/* strtoimax (C17 7.8.2.3): strtoll, intmax_t being a 64-bit type as long
   long is. */
#include <inttypes.h>
#include <axc/text.h>

intmax_t strtoimax(const char *restrict nptr, char **restrict endptr, int base) {
    return (intmax_t)strtoll(nptr, endptr, base);
}

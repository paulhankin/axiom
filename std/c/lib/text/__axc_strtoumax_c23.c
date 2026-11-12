/* __axc_strtoumax_c23: strtoumax in C23 mode, whose header maps the name here: the
   0b and 0B prefixes too, for base 0 and 2 (C23 7.24.1.7). */
#include <inttypes.h>
#include <axc/text.h>
#include <stdlib.h>

uintmax_t __axc_strtoumax_c23(const char *restrict nptr, char **restrict endptr, int base) {
    return (uintmax_t)__axc_strtoull_c23(nptr, endptr, base);
}

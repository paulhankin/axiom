/* __axc_strtoimax_c23: strtoimax in C23 mode, whose header maps the name here: the
   0b and 0B prefixes too, for base 0 and 2 (C23 7.24.1.7). */
#include <inttypes.h>
#include <axc/text.h>
#include <stdlib.h>

intmax_t __axc_strtoimax_c23(const char *restrict nptr, char **restrict endptr, int base) {
    return (intmax_t)__axc_strtoll_c23(nptr, endptr, base);
}

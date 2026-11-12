/* strfromf (<stdlib.h>, C23 7.24.1.3): fp formatted as snprintf(s, n,
   format, (double)fp) would, for a format "%[.precision]conversion" with
   a conversion of a, A, e, E, f, F, g or G (__axc_strfrom.c). */
#include <stdlib.h>
#include "axc/core_impl.h"

int strfromf(char *restrict s, size_t n, const char *restrict format, float fp) {
    return __axc_strfrom(s, n, format, fp, 0);
}

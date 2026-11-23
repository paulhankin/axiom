/* strfromd64 (<stdlib.h>, C23 7.24.1.4): fp formatted as snprintf(s, n,
   format, fp) would with the length modifier of _Decimal64, for a format
   "%[.precision]conversion" with a conversion of a, A, e, E, f, F, g or G
   (__axc_strfrom_dec.c). */
#include <stdlib.h>

int __axc_strfrom_dec(char *restrict s, size_t n, const char *restrict format, const void *x, int dfmt);

int strfromd64(char *restrict s, size_t n, const char *restrict format, __axiom_decimal64 fp) {
    return __axc_strfrom_dec(s, n, format, &fp, 2);
}

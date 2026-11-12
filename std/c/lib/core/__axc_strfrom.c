/* __axc_strfrom: strfromd, strfromf and strfroml's common part (C23
   7.24.1.3): checks that format is "%[.precision]conversion", with a
   conversion of a, A, e, E, f, F, g or G (else nothing is written and the
   result is -1, where the standard leaves it undefined), and formats the
   value as snprintf does (the standard I/O's formatting: one engine for
   every conversion), into at most n bytes at s; the length the whole
   result has. long_double adds the L length modifier. */
#include <stdarg.h>
#include "axc/core_impl.h"

int snprintf(char *restrict s, size_t n, const char *restrict format, ...);

int __axc_strfrom(char *restrict s, size_t n, const char *restrict format, long double fp, int long_double) {
    char f[16];
    size_t k = 0, i = 1;
    if (format[0] != '%')
        return -1;
    f[k++] = '%';
    if (format[i] == '.') {
        f[k++] = format[i++];
        while (format[i] >= '0' && format[i] <= '9') {
            if (k >= 12)
                return -1;
            f[k++] = format[i++];
        }
    }
    switch (format[i]) {
    case 'a': case 'A': case 'e': case 'E': case 'f': case 'F': case 'g': case 'G':
        break;
    default:
        return -1;
    }
    if (format[i + 1] != 0)
        return -1;
    if (long_double)
        f[k++] = 'L';
    f[k++] = format[i];
    f[k] = 0;
    if (long_double)
        return snprintf(s, n, f, fp);
    return snprintf(s, n, f, (double)fp);
}

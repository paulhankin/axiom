/* __axc_strfrom_dec: strfromd32, strfromd64 and strfromd128's common part
   (C23 7.24.1.4): as __axc_strfrom (format "%[.precision]conversion", a
   conversion of a, A, e, E, f, F, g or G, else -1), with the length
   modifier of the decimal floating type dfmt (1 H, 2 D, 3 DD) and the value
   at x. */
#include <stddef.h>

int snprintf(char *restrict s, size_t n, const char *restrict format, ...);

int __axc_strfrom_dec(char *restrict s, size_t n, const char *restrict format, const void *x, int dfmt) {
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
    if (dfmt == 1) {
        f[k++] = 'H';
    } else {
        f[k++] = 'D';
        if (dfmt == 3)
            f[k++] = 'D';
    }
    f[k++] = format[i];
    f[k] = 0;
    if (dfmt == 1)
        return snprintf(s, n, f, *(const __axiom_decimal32 *)x);
    if (dfmt == 2)
        return snprintf(s, n, f, *(const __axiom_decimal64 *)x);
    return snprintf(s, n, f, *(const __axiom_decimal128 *)x);
}

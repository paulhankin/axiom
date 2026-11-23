/* __axc_wcstoll_c23: wcstoll in C23 mode, whose header maps the name here: the
   0b and 0B prefixes too, for base 0 and 2 (C23 7.24.1.7). */
#include <axc/text.h>
#include <stdlib.h>
#include <wchar.h>

long long __axc_wcstoll_c23(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base) {
    char buf[64], *end;
    size_t lead;
    char *p = __axc_wcs_narrow(nptr, buf, sizeof buf, &lead);
    long long v;
    if (!p) {
        if (endptr)
            *endptr = (wchar_t *)nptr;
        return 0;
    }
    v = __axc_strtoll_c23(p, &end, base);
    if (endptr)
        *endptr = (wchar_t *)(end == p ? nptr : nptr + lead + (end - p));
    __axc_wcs_narrow_done(p, buf);
    return v;
}

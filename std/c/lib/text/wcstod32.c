/* wcstod32 (C23 7.31.4.1.2): strtod32 of the wide string
   (__axc_wcs_narrow); the end of the subject sequence is the same character
   in both strings. */
#include <axc/text.h>
#include <wchar.h>

__axiom_decimal32 strtod32(const char *restrict nptr, char **restrict endptr);

__axiom_decimal32 wcstod32(const wchar_t *restrict nptr, wchar_t **restrict endptr) {
    char buf[64], *end;
    size_t lead;
    char *p = __axc_wcs_narrow(nptr, buf, sizeof buf, &lead);
    __axiom_decimal32 v;
    if (!p) {
        if (endptr)
            *endptr = (wchar_t *)nptr;
        return strtod32("", 0);
    }
    v = strtod32(p, &end);
    if (endptr)
        *endptr = (wchar_t *)(end == p ? nptr : nptr + lead + (end - p));
    __axc_wcs_narrow_done(p, buf);
    return v;
}

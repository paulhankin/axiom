/* wcstof (C17 7.29.4.1.1): strtof of the wide string (__axc_wcs_narrow); the
   end of the subject sequence is the same character in both strings. */
#include <axc/text.h>
#include <wchar.h>

float wcstof(const wchar_t *restrict nptr, wchar_t **restrict endptr) {
    char buf[64], *end;
    size_t lead;
    char *p = __axc_wcs_narrow(nptr, buf, sizeof buf, &lead);
    float v;
    if (!p) {
        if (endptr)
            *endptr = (wchar_t *)nptr;
        return 0;
    }
    v = strtof(p, &end);
    if (endptr)
        *endptr = (wchar_t *)(end == p ? nptr : nptr + lead + (end - p));
    __axc_wcs_narrow_done(p, buf);
    return v;
}

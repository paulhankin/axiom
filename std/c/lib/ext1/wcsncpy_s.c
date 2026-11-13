/* wcsncpy_s (C11 K.3.9.2.1.2): copies at most n wide characters of the wide string s2
   (none after its null) to s1, an array of s1max, and a null wide character
   after them if none was copied. Runtime-constraints: neither pointer
   null, s1max and n at most RSIZE_MAX, s1max not 0, s2's length less than
   s1max when n is not less than s1max, the objects not overlapping; after
   a violation s1[0] is the null wide character (if s1 is not null and s1max is
   1 to RSIZE_MAX). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>

errno_t wcsncpy_s(wchar_t *__AXC_RESTRICT s1, rsize_t s1max, const wchar_t *__AXC_RESTRICT s2, rsize_t n) {
    const char *what = NULL;
    errno_t e = EINVAL;
    size_t k = 0;
    if (s1 == NULL)
        what = "s1 is a null pointer";
    else if (s2 == NULL)
        what = "s2 is a null pointer";
    else if (s1max > RSIZE_MAX)
        what = "s1max is greater than RSIZE_MAX", e = ERANGE;
    else if (n > RSIZE_MAX)
        what = "n is greater than RSIZE_MAX", e = ERANGE;
    else if (s1max == 0)
        what = "s1max is zero", e = ERANGE;
    else if (n >= s1max && wcsnlen_s(s2, s1max) >= s1max)
        what = "s2 does not fit in s1max wide characters", e = ERANGE;
    else if (k = wcsnlen_s(s2, n), __axc_ext1_overlap(s1, (k + 1) * sizeof(wchar_t), s2, (k < n ? k + 1 : k) * sizeof(wchar_t)))
        what = "the objects overlap";
    if (what != NULL) {
        if (s1 != NULL && s1max > 0 && s1max <= RSIZE_MAX)
            s1[0] = 0;
        __axc_ext1_violation("wcsncpy_s", what, e);
        return e;
    }
    wmemcpy(s1, s2, k);
    s1[k] = 0;
    return 0;
}

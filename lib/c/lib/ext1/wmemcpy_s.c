/* wmemcpy_s (C11 K.3.9.2.1.3): copies n wide characters from s2 to s1, an array of
   s1max. Runtime-constraints: neither pointer null, s1max and n at most
   RSIZE_MAX, n at most s1max, the objects not overlapping; after a
   violation the first s1max wide characters of s1 are zero (if s1 is not null
   and s1max at most RSIZE_MAX). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>

errno_t wmemcpy_s(wchar_t *__AXC_RESTRICT s1, rsize_t s1max, const wchar_t *__AXC_RESTRICT s2, rsize_t n) {
    const char *what = NULL;
    errno_t e = EINVAL;
    if (s1 == NULL)
        what = "s1 is a null pointer";
    else if (s2 == NULL)
        what = "s2 is a null pointer";
    else if (s1max > RSIZE_MAX)
        what = "s1max is greater than RSIZE_MAX", e = ERANGE;
    else if (n > RSIZE_MAX)
        what = "n is greater than RSIZE_MAX", e = ERANGE;
    else if (n > s1max)
        what = "n is greater than s1max", e = ERANGE;
    else if (__axc_ext1_overlap(s1, (n) * sizeof(wchar_t), s2, (n) * sizeof(wchar_t)))
        what = "the objects overlap";
    if (what != NULL) {
        if (s1 != NULL && s1max <= RSIZE_MAX)
            wmemset(s1, 0, s1max);
        __axc_ext1_violation("wmemcpy_s", what, e);
        return e;
    }
    wmemcpy(s1, s2, n);
    return 0;
}

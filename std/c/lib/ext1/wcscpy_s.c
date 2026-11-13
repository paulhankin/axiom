/* wcscpy_s (C11 K.3.9.2.1.1): copies the wide string s2 (with its null) to s1, an
   array of s1max. Runtime-constraints: neither pointer null, s1max not 0
   and at most RSIZE_MAX, s2's length less than s1max, the objects not
   overlapping; after a violation s1[0] is the null wide character (if s1 is not
   null and s1max is 1 to RSIZE_MAX). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>

errno_t wcscpy_s(wchar_t *__AXC_RESTRICT s1, rsize_t s1max, const wchar_t *__AXC_RESTRICT s2) {
    const char *what = NULL;
    errno_t e = EINVAL;
    size_t n = 0;
    if (s1 == NULL)
        what = "s1 is a null pointer";
    else if (s2 == NULL)
        what = "s2 is a null pointer";
    else if (s1max > RSIZE_MAX)
        what = "s1max is greater than RSIZE_MAX", e = ERANGE;
    else if (s1max == 0)
        what = "s1max is zero", e = ERANGE;
    else if ((n = wcsnlen_s(s2, s1max)) >= s1max)
        what = "s2 does not fit in s1max wide characters", e = ERANGE;
    else if (__axc_ext1_overlap(s1, (n + 1) * sizeof(wchar_t), s2, (n + 1) * sizeof(wchar_t)))
        what = "the objects overlap";
    if (what != NULL) {
        if (s1 != NULL && s1max > 0 && s1max <= RSIZE_MAX)
            s1[0] = 0;
        __axc_ext1_violation("wcscpy_s", what, e);
        return e;
    }
    wmemcpy(s1, s2, n + 1);
    return 0;
}

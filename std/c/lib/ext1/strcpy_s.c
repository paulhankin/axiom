/* strcpy_s (C11 K.3.7.1.3): copies the string s2 (with its null) to s1, an
   array of s1max. Runtime-constraints: neither pointer null, s1max not 0
   and at most RSIZE_MAX, s2's length less than s1max, the objects not
   overlapping; after a violation s1[0] is the null character (if s1 is not
   null and s1max is 1 to RSIZE_MAX). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <string.h>

errno_t strcpy_s(char *__AXC_RESTRICT s1, rsize_t s1max, const char *__AXC_RESTRICT s2) {
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
    else if ((n = strnlen_s(s2, s1max)) >= s1max)
        what = "s2 does not fit in s1max characters", e = ERANGE;
    else if (__axc_ext1_overlap(s1, n + 1, s2, n + 1))
        what = "the objects overlap";
    if (what != NULL) {
        if (s1 != NULL && s1max > 0 && s1max <= RSIZE_MAX)
            s1[0] = 0;
        __axc_ext1_violation("strcpy_s", what, e);
        return e;
    }
    memcpy(s1, s2, n + 1);
    return 0;
}

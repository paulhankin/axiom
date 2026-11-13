/* strncat_s (C11 K.3.7.2.2): appends at most n characters of the string s2
   (none after its null), and a null character, to the string in s1, an
   array of s1max. With m = s1max - strnlen_s(s1, s1max), the runtime-
   constraints: neither pointer null, s1max and n at most RSIZE_MAX, s1max
   not 0, m not 0, s2's length less than m when n is not less than m, the
   objects not overlapping; after a violation s1[0] is the null character
   (if s1 is not null and s1max is 1 to RSIZE_MAX). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <string.h>

errno_t strncat_s(char *__AXC_RESTRICT s1, rsize_t s1max, const char *__AXC_RESTRICT s2, rsize_t n) {
    const char *what = NULL;
    errno_t e = EINVAL;
    size_t len = 0, m = 0, k = 0;
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
    else if ((m = s1max - (len = strnlen_s(s1, s1max))) == 0)
        what = "s1 has no null character in s1max characters", e = ERANGE;
    else if (n >= m && strnlen_s(s2, m) >= m)
        what = "s2 does not fit after s1's string", e = ERANGE;
    else if (k = strnlen_s(s2, n), __axc_ext1_overlap(s1, len + k + 1, s2, k < n ? k + 1 : k))
        what = "the objects overlap";
    if (what != NULL) {
        if (s1 != NULL && s1max > 0 && s1max <= RSIZE_MAX)
            s1[0] = 0;
        __axc_ext1_violation("strncat_s", what, e);
        return e;
    }
    memcpy(s1 + len, s2, k);
    s1[len + k] = 0;
    return 0;
}

/* strerror_s (C11 K.3.7.4.2): strerror's message for errnum into s, an
   array of maxsize: all of it, or (nonzero result) as much as fits before
   a null character, its last three characters "..." when maxsize is more
   than 3. Runtime-constraints: s not null, maxsize not 0 and at most
   RSIZE_MAX; after a violation s is unchanged. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <string.h>

errno_t strerror_s(char *s, rsize_t maxsize, errno_t errnum) {
    const char *msg, *what = NULL;
    size_t n;
    if (s == NULL)
        what = "s is a null pointer";
    else if (maxsize > RSIZE_MAX)
        what = "maxsize is greater than RSIZE_MAX";
    else if (maxsize == 0)
        what = "maxsize is zero";
    if (what != NULL) {
        __axc_ext1_violation("strerror_s", what, s == NULL ? EINVAL : ERANGE);
        return s == NULL ? EINVAL : ERANGE;
    }
    msg = strerror(errnum);
    n = strlen(msg);
    if (n < maxsize) {
        memcpy(s, msg, n + 1);
        return 0;
    }
    memcpy(s, msg, maxsize - 1);
    s[maxsize - 1] = 0;
    if (maxsize > 3)
        memcpy(s + maxsize - 4, "...", 3);
    return ERANGE;
}

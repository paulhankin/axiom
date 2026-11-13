/* tmpnam_s (C11 K.3.5.1.2): tmpnam's kind of name (random, in the
   directory for temporary files, of no file that exists) into s, an array
   of maxsize. Runtime-constraints: s not null, maxsize at most RSIZE_MAX
   and greater than the name's length; after a violation, or when no name
   can be made, s[0] is the null character (s not null, maxsize 1 to
   RSIZE_MAX) and the result nonzero. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <stdio.h>
#include <string.h>

errno_t tmpnam_s(char *s, rsize_t maxsize) {
    char name[L_tmpnam];
    const char *what = NULL;
    errno_t e = ERANGE;
    size_t n = 0;
    if (s == NULL)
        what = "s is a null pointer", e = EINVAL;
    else if (maxsize > RSIZE_MAX)
        what = "maxsize is greater than RSIZE_MAX";
    else if (tmpnam(name) == NULL) {
        if (maxsize > 0)
            s[0] = 0;
        return EEXIST;
    } else if ((n = strlen(name)) >= maxsize)
        what = "the name does not fit in maxsize characters";
    if (what != NULL) {
        if (s != NULL && maxsize > 0 && maxsize <= RSIZE_MAX)
            s[0] = 0;
        __axc_ext1_violation("tmpnam_s", what, e);
        return e;
    }
    memcpy(s, name, n + 1);
    return 0;
}

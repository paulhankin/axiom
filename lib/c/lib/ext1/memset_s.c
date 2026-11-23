/* memset_s (C11 K.3.7.4.1): stores c (converted to unsigned char) into the
   first n characters of s, an array of smax; the stores are made whatever
   the program does with the array later (memset_explicit's). Runtime-
   constraints: s not null, smax and n at most RSIZE_MAX, n at most smax;
   after a violation the first smax characters of s are c (if s is not
   null and smax at most RSIZE_MAX). */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <string.h>

/* (C23's, declared by <string.h> in C23 mode only) */
void *memset_explicit(void *s, int c, size_t n);

errno_t memset_s(void *s, rsize_t smax, int c, rsize_t n) {
    const char *what = NULL;
    errno_t e = EINVAL;
    if (s == NULL)
        what = "s is a null pointer";
    else if (smax > RSIZE_MAX)
        what = "smax is greater than RSIZE_MAX", e = ERANGE;
    else if (n > RSIZE_MAX)
        what = "n is greater than RSIZE_MAX", e = ERANGE;
    else if (n > smax)
        what = "n is greater than smax", e = ERANGE;
    if (what != NULL) {
        if (s != NULL && smax <= RSIZE_MAX)
            memset_explicit(s, c, smax);
        __axc_ext1_violation("memset_s", what, e);
        return e;
    }
    memset_explicit(s, c, n);
    return 0;
}

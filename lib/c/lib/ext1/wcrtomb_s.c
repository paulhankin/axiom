/* wcrtomb_s (C11 K.3.9.3.1.1): wcrtomb with the bounds of s (an array of
   smax): the multibyte character of wc into s and its length into
   *retval; s null is as wcrtomb(buf, L'\0', ps) with a buffer of its own.
   Runtime-constraints: retval and ps not null; s not null: smax 1 to
   RSIZE_MAX and at least the character's length; s null: smax 0. After a
   violation s[0] is the null character (s not null, smax 1 to RSIZE_MAX),
   *retval (size_t)-1 (retval not null), and *ps unchanged. An encoding
   error stores (size_t)-1 into *retval and returns EILSEQ. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>
#include <string.h>

errno_t wcrtomb_s(size_t *__AXC_RESTRICT retval, char *__AXC_RESTRICT s, rsize_t smax, wchar_t wc,
                  mbstate_t *__AXC_RESTRICT ps) {
    const char *what = NULL;
    errno_t e = EINVAL;
    char buf[16];
    mbstate_t t;
    size_t n = 0;
    if (retval == NULL)
        what = "retval is a null pointer";
    else if (ps == NULL)
        what = "ps is a null pointer";
    else if (s != NULL && smax == 0)
        what = "smax is zero", e = ERANGE;
    else if (s != NULL && smax > RSIZE_MAX)
        what = "smax is greater than RSIZE_MAX", e = ERANGE;
    else if (s == NULL && smax != 0)
        what = "s is a null pointer and smax is not zero";
    if (what == NULL) {
        t = *ps;
        n = wcrtomb(buf, s != NULL ? wc : 0, &t);
        if (n == (size_t)-1) {
            *ps = t;
            *retval = (size_t)-1;
            return EILSEQ;
        }
        if (s != NULL && n > smax)
            what = "the multibyte character does not fit in smax bytes", e = ERANGE;
    }
    if (what != NULL) {
        if (s != NULL && smax > 0 && smax <= RSIZE_MAX)
            s[0] = 0;
        if (retval != NULL)
            *retval = (size_t)-1;
        __axc_ext1_violation("wcrtomb_s", what, e);
        return e;
    }
    if (s != NULL)
        memcpy(s, buf, n);
    *ps = t;
    *retval = n;
    return 0;
}

/* wctomb_s (C11 K.3.6.4.1): the multibyte character of wc into s (an array
   of smax) and its length into *status (-1, and the result EILSEQ, if wc
   has none); s null: *status 0 (no encoding has shift states) and the
   function's own conversion state made the initial one (it keeps a
   surrogate's first half where wchar_t has 16 bits). Runtime-constraints:
   status not null; s not null: smax at most RSIZE_MAX and at least the
   character's length; s null: smax 0. After a violation *status and s
   are unchanged. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>
#include <string.h>

errno_t wctomb_s(int *__AXC_RESTRICT status, char *__AXC_RESTRICT s, rsize_t smax, wchar_t wc) {
    static _Thread_local mbstate_t st;
    const char *what = NULL;
    errno_t e = EINVAL;
    char buf[16];
    mbstate_t t;
    size_t n = 0;
    if (status == NULL)
        what = "status is a null pointer";
    else if (s == NULL && smax != 0)
        what = "s is a null pointer and smax is not zero";
    else if (s != NULL && smax > RSIZE_MAX)
        what = "smax is greater than RSIZE_MAX", e = ERANGE;
    if (what == NULL && s == NULL) {
        memset(&st, 0, sizeof st);
        *status = 0;
        return 0;
    }
    if (what == NULL) {
        t = st;
        n = wcrtomb(buf, wc, &t);
        if (n == (size_t)-1) {
            memset(&st, 0, sizeof st);
            *status = -1;
            return EILSEQ;
        }
        if (n > smax)
            what = "the multibyte character does not fit in smax bytes", e = ERANGE;
    }
    if (what != NULL) {
        __axc_ext1_violation("wctomb_s", what, e);
        return e;
    }
    memcpy(s, buf, n);
    st = t;
    *status = (int)n;
    return 0;
}

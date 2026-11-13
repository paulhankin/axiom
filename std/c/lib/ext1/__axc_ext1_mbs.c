/* __axc_ext1_mbs - the conversion of Annex K's mbstowcs_s and mbsrtowcs_s
   (C11 K.3.6.5.1, K.3.9.3.2.1), after their own pointer checks: the
   runtime-constraints on dst, dstmax and len (dst not null: len and
   dstmax at most RSIZE_MAX / sizeof(wchar_t), dstmax not 0, and when len
   is not less than dstmax the string's null character within dstmax wide
   characters; dst null: dstmax 0), then mbsrtowcs's conversion of at most
   len wide characters (dstmax when len is not less), with a null wide
   character after them if the conversion did not reach the string's. An
   encoding error stores (size_t)-1 into *retval and returns EILSEQ. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>

errno_t __axc_ext1_mbs(const char *fn, size_t *retval, wchar_t *dst, rsize_t dstmax, const char **src, rsize_t len,
                       mbstate_t *ps) {
    const size_t lim = RSIZE_MAX / sizeof(wchar_t);
    const char *what = NULL;
    errno_t e = ERANGE;
    size_t n;
    if (dst != NULL) {
        if (len > lim)
            what = "len is greater than RSIZE_MAX / sizeof(wchar_t)";
        else if (dstmax > lim)
            what = "dstmax is greater than RSIZE_MAX / sizeof(wchar_t)";
        else if (dstmax == 0)
            what = "dstmax is zero";
        else if (len >= dstmax) {
            /* (the null character must come within dstmax: counted first) */
            mbstate_t t = *ps;
            const char *q = *src;
            n = mbsrtowcs(NULL, &q, 0, &t);
            if (n != (size_t)-1 && n >= dstmax)
                what = "no null character within dstmax wide characters of the source";
        }
    } else if (dstmax != 0) {
        what = "dst is a null pointer and dstmax is not zero";
        e = EINVAL;
    }
    if (what != NULL) {
        *retval = (size_t)-1;
        if (dst != NULL && dstmax > 0 && dstmax <= lim)
            dst[0] = 0;
        __axc_ext1_violation(fn, what, e);
        return e;
    }
    if (dst == NULL) {
        n = mbsrtowcs(NULL, src, 0, ps);
    } else {
        n = mbsrtowcs(dst, src, len < dstmax ? len : dstmax, ps);
        if (n != (size_t)-1 && *src != NULL)
            dst[n] = 0;
    }
    *retval = n;
    if (n == (size_t)-1) {
        if (dst != NULL)
            dst[0] = 0;
        return EILSEQ;
    }
    return 0;
}

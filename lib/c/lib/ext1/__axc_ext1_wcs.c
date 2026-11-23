/* __axc_ext1_wcs - the conversion of Annex K's wcstombs_s and wcsrtombs_s
   (C11 K.3.6.5.2, K.3.9.3.2.2), after their own pointer checks: the
   runtime-constraints on dst, dstmax and len (dst not null: len and
   dstmax at most RSIZE_MAX, dstmax not 0, and when len is not less than
   dstmax the whole conversion, with its null character, within dstmax
   bytes; dst null: dstmax 0), then the conversion: the wide characters
   whose multibyte characters fit in min(len, dstmax - 1) bytes (the null
   character in min(len, dstmax)), and a null character after them if the
   string's was not reached; *src as wcsrtombs leaves it. An encoding
   error stores (size_t)-1 into *retval and returns EILSEQ. */
#define __STDC_WANT_LIB_EXT1__ 1
#include <axc/ext1_impl.h>
#include <wchar.h>
#include <string.h>

errno_t __axc_ext1_wcs(const char *fn, size_t *retval, char *dst, rsize_t dstmax, const wchar_t **src, rsize_t len,
                       mbstate_t *ps) {
    const char *what = NULL;
    errno_t e = ERANGE;
    size_t n, total = 0, lim, lim0;
    const wchar_t *p;
    char buf[16];
    if (dst != NULL) {
        if (len > RSIZE_MAX)
            what = "len is greater than RSIZE_MAX";
        else if (dstmax > RSIZE_MAX)
            what = "dstmax is greater than RSIZE_MAX";
        else if (dstmax == 0)
            what = "dstmax is zero";
        else if (len >= dstmax) {
            mbstate_t t = *ps;
            const wchar_t *q = *src;
            n = wcsrtombs(NULL, &q, 0, &t);
            if (n != (size_t)-1 && n >= dstmax)
                what = "the conversion does not fit in dstmax bytes";
        }
    } else if (dstmax != 0) {
        what = "dst is a null pointer and dstmax is not zero";
        e = EINVAL;
    }
    if (what != NULL) {
        *retval = (size_t)-1;
        if (dst != NULL && dstmax > 0 && dstmax <= RSIZE_MAX)
            dst[0] = 0;
        __axc_ext1_violation(fn, what, e);
        return e;
    }
    if (dst == NULL) {
        n = wcsrtombs(NULL, src, 0, ps);
        *retval = n;
        return n == (size_t)-1 ? EILSEQ : 0;
    }
    lim0 = len < dstmax ? len : dstmax;
    lim = len < dstmax - 1 ? len : dstmax - 1;
    for (p = *src;; p++) {
        mbstate_t save = *ps;
        size_t k = wcrtomb(buf, *p, ps);
        if (k == (size_t)-1) {
            *src = p;
            *retval = (size_t)-1;
            dst[0] = 0;
            return EILSEQ;
        }
        if (*p == 0) {
            if (total + k > lim0) {
                *ps = save;
                break;
            }
            memcpy(dst + total, buf, k);
            *src = NULL;
            *retval = total + k - 1;
            return 0;
        }
        if (total + k > lim) {
            *ps = save;
            break;
        }
        memcpy(dst + total, buf, k);
        total += k;
    }
    *src = p;
    dst[total] = 0;
    *retval = total;
    return 0;
}

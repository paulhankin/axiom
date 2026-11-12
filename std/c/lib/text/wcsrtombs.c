/* wcsrtombs (C17 7.29.6.4.2): wcrtomb over a wide string; it stops before
   a character whose bytes would go beyond len. Where wchar_t has 16 bits a
   surrogate pair is one character, and a surrogate out of a pair EILSEQ. */
#include <axc/text.h>
#include <errno.h>
#include <wchar.h>

size_t wcsrtombs(char *restrict dst, const wchar_t **restrict src, size_t len, mbstate_t *restrict ps) {
    static mbstate_t internal;
    const wchar_t *w = *src;
    size_t k = 0;
    if (!ps)
        ps = &internal;
    for (;;) {
        char buf[4];
        unsigned long c = (unsigned long)*w;
        int used = 1, n;
        if (c < 0x80 && !ps->__axiom_high) {
            /* (ASCII, in every encoding: the common case first) */
            if (dst) {
                if (k >= len)
                    break;
                dst[k] = (char)c;
            }
            if (c == 0) {
                if (dst)
                    *src = 0;
                return k;
            }
            k++;
            w++;
            continue;
        }
#if __AXIOM_WCHAR_WIDTH__ == 16
        c &= 0xffff;
        if (ps->__axiom_high) {
            /* (a high surrogate that wcrtomb kept: this is its pair's
               second half) */
            if (c < 0xdc00 || c > 0xdfff) {
                errno = EILSEQ;
                if (dst)
                    *src = w;
                return (size_t)-1;
            }
            c = 0x10000 + ((ps->__axiom_out - 0xd800) << 10) + (c - 0xdc00);
        } else if (c >= 0xd800 && c <= 0xdbff && __axc_mb_utf8()) {
            unsigned long d = (unsigned long)w[1] & 0xffff;
            if (d < 0xdc00 || d > 0xdfff) {
                errno = EILSEQ;
                if (dst)
                    *src = w;
                return (size_t)-1;
            }
            c = 0x10000 + ((c - 0xd800) << 10) + (d - 0xdc00);
            used = 2;
        }
#endif
        n = __axc_mb_put(buf, c);
        if (n < 0) {
            errno = EILSEQ;
            if (dst)
                *src = w;
            return (size_t)-1;
        }
        if (dst) {
            int i;
            if (len - k < (size_t)n)
                break;
            for (i = 0; i < n; i++)
                dst[k + i] = buf[i];
        }
        ps->__axiom_high = 0;
        if (c == 0) {
            if (dst)
                *src = 0;
            return k;
        }
        k += (size_t)n;
        w += used;
    }
    *src = w;
    return k;
}

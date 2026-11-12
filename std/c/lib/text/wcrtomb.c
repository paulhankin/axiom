/* wcrtomb (C17 7.29.6.3.3). Where wchar_t has 16 bits (UTF-16), a high
   surrogate is kept in *ps (no bytes, 0 returned) until the low one that
   completes the character comes; a surrogate out of its pair is EILSEQ. (A negative wchar_t is beyond
   U+10FFFF as an unsigned long: EILSEQ.) */
#include <axc/text.h>
#include <errno.h>
#include <wchar.h>

size_t wcrtomb(char *restrict s, wchar_t wc, mbstate_t *restrict ps) {
    static mbstate_t internal;
    char buf[4];
    unsigned long c;
    int n;
    if (!ps)
        ps = &internal;
    if (!s) {
        s = buf;
        wc = 0;
    }
    c = (unsigned long)wc;
#if __AXIOM_WCHAR_WIDTH__ == 16
    c &= 0xffff;
    if (ps->__axiom_high) {
        if (c < 0xdc00 || c > 0xdfff) {
            ps->__axiom_high = 0;
            errno = EILSEQ;
            return (size_t)-1;
        }
        c = 0x10000 + ((ps->__axiom_out - 0xd800) << 10) + (c - 0xdc00);
        ps->__axiom_high = 0;
    } else if (c >= 0xd800 && c <= 0xdbff && __axc_mb_utf8()) {
        ps->__axiom_out = c;
        ps->__axiom_high = 1;
        return 0;
    }
#endif
    n = __axc_mb_put(s, c);
    if (n < 0) {
        errno = EILSEQ;
        return (size_t)-1;
    }
    return (size_t)n;
}

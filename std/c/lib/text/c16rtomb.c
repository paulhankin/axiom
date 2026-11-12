/* c16rtomb (C17 7.28.1.2): UTF-16 code units to the locale's encoding. A
   high surrogate is kept in *ps (0 bytes) until the low one that completes
   the character; a surrogate out of a pair is EILSEQ. */
#include <axc/text.h>
#include <errno.h>
#include <uchar.h>

size_t c16rtomb(char *restrict s, char16_t c16, mbstate_t *restrict ps) {
    static mbstate_t internal;
    char buf[4];
    unsigned long c = c16;
    int n;
    if (!ps)
        ps = &internal;
    if (!s) {
        s = buf;
        c = 0;
    }
    if (ps->__axiom_high) {
        ps->__axiom_high = 0;
        if (c < 0xdc00 || c > 0xdfff) {
            errno = EILSEQ;
            return (size_t)-1;
        }
        c = 0x10000 + ((ps->__axiom_out - 0xd800) << 10) + (c - 0xdc00);
    } else if (c >= 0xd800 && c <= 0xdbff) {
        ps->__axiom_out = c;
        ps->__axiom_high = 1;
        return 0;
    }
    n = __axc_mb_put(s, c);
    if (n < 0) {
        errno = EILSEQ;
        return (size_t)-1;
    }
    return (size_t)n;
}

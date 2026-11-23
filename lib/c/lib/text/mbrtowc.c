/* mbrtowc (C17 7.29.6.3.2). Where wchar_t has 16 bits (UTF-16), a
   character beyond U+FFFF is two wchar_t: the call that reads it stores
   the high surrogate and keeps the low one in *ps, and the next call
   stores that and returns (size_t)-3, reading nothing, as mbrtoc16 does
   (7.28.1.1p3). */
#include <axc/text.h>
#include <wchar.h>

size_t mbrtowc(wchar_t *restrict pwc, const char *restrict s, size_t n, mbstate_t *restrict ps) {
    static _Thread_local mbstate_t internal; /* (each thread's: free of data races) */
    unsigned long c;
    size_t r;
    if (!ps)
        ps = &internal;
    if (!s) {
        pwc = 0;
        s = "";
        n = 1;
    }
    if (ps->__axiom_npend) {
        if (pwc)
            *pwc = (wchar_t)ps->__axiom_out;
        ps->__axiom_npend = 0;
        return (size_t)-3;
    }
    r = __axc_mb_get(&c, s, n, ps);
    if (r == (size_t)-1 || r == (size_t)-2)
        return r;
#if __AXIOM_WCHAR_WIDTH__ == 16
    if (c > 0xffff) {
        if (pwc) {
            *pwc = (wchar_t)(0xd800 + ((c - 0x10000) >> 10));
            ps->__axiom_out = 0xdc00 + (c & 0x3ff);
            ps->__axiom_npend = 1;
        }
        return r;
    }
#endif
    if (pwc)
        *pwc = (wchar_t)c;
    return r;
}

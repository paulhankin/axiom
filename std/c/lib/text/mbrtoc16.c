/* mbrtoc16 (C17 7.28.1.1): as mbrtowc, to UTF-16: a character beyond
   U+FFFF is its high surrogate, and the next call gives the low one
   ((size_t)-3, reading nothing). */
#include <axc/text.h>
#include <uchar.h>

size_t mbrtoc16(char16_t *restrict pc16, const char *restrict s, size_t n, mbstate_t *restrict ps) {
    static _Thread_local mbstate_t internal; /* (each thread's: free of data races) */
    unsigned long c;
    size_t r;
    if (!ps)
        ps = &internal;
    if (!s) {
        pc16 = 0;
        s = "";
        n = 1;
    }
    if (ps->__axiom_npend) {
        if (pc16)
            *pc16 = (char16_t)ps->__axiom_out;
        ps->__axiom_npend = 0;
        return (size_t)-3;
    }
    r = __axc_mb_get(&c, s, n, ps);
    if (r == (size_t)-1 || r == (size_t)-2)
        return r;
    if (c > 0xffff) {
        ps->__axiom_out = 0xdc00 + (c & 0x3ff);
        ps->__axiom_npend = 1;
        c = 0xd800 + ((c - 0x10000) >> 10);
    }
    if (pc16)
        *pc16 = (char16_t)c;
    return r;
}

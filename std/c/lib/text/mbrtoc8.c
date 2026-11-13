/* mbrtoc8 (C23 7.30.2.2): as mbrtowc, to UTF-8 code units: the call that
   reads a character gives its first unit, and the following calls the
   others, each returning (size_t)-3 and reading nothing. (char8_t is C23's
   unsigned char: this source is C17.) */
#include <axc/text.h>
#include <uchar.h>

size_t mbrtoc8(unsigned char *restrict pc8, const char *restrict s, size_t n, mbstate_t *restrict ps) {
    static _Thread_local mbstate_t internal; /* (each thread's: free of data races) */
    unsigned long c;
    size_t r;
    char u[4];
    int k, i;
    if (!ps)
        ps = &internal;
    if (!s) {
        pc8 = 0;
        s = "";
        n = 1;
    }
    if (ps->__axiom_npend) {
        /* (the units still to give, the next in the low byte) */
        if (pc8)
            *pc8 = (unsigned char)(ps->__axiom_out & 0xff);
        ps->__axiom_out >>= 8;
        ps->__axiom_npend--;
        return (size_t)-3;
    }
    r = __axc_mb_get(&c, s, n, ps);
    if (r == (size_t)-1 || r == (size_t)-2)
        return r;
    k = __axc_utf8_put(u, c);
    ps->__axiom_out = 0;
    for (i = k - 1; i >= 1; i--)
        ps->__axiom_out = (ps->__axiom_out << 8) | (unsigned char)u[i];
    ps->__axiom_npend = (unsigned char)(k - 1);
    if (pc8)
        *pc8 = (unsigned char)u[0];
    return r;
}

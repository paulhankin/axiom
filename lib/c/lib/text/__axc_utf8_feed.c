/* __axc_utf8_feed - one byte of UTF-8 into a decoding state (axc/text.h).
   The well-formed sequences are those of the Unicode Standard's table 3-7
   (RFC 3629): no overlong forms, no surrogates, nothing beyond U+10FFFF;
   a byte that cannot follow is rejected as soon as it is seen. */
#include <axc/text.h>

long __axc_utf8_feed(mbstate_t *ps, unsigned char b) {
    if (ps->__axiom_need == 0) {
        ps->__axiom_lo = 0x80;
        ps->__axiom_hi = 0xbf;
        if (b < 0x80)
            return b;
        if (b < 0xc2 || b > 0xf4)
            return -1;
        if (b < 0xe0) {
            ps->__axiom_need = 1;
            ps->__axiom_value = b & 0x1f;
        } else if (b < 0xf0) {
            ps->__axiom_need = 2;
            ps->__axiom_value = b & 0x0f;
            if (b == 0xe0)
                ps->__axiom_lo = 0xa0; /* (no overlong forms) */
            else if (b == 0xed)
                ps->__axiom_hi = 0x9f; /* (no surrogates) */
        } else {
            ps->__axiom_need = 3;
            ps->__axiom_value = b & 0x07;
            if (b == 0xf0)
                ps->__axiom_lo = 0x90; /* (no overlong forms) */
            else if (b == 0xf4)
                ps->__axiom_hi = 0x8f; /* (nothing beyond U+10FFFF) */
        }
        return -2;
    }
    if (b < ps->__axiom_lo || b > ps->__axiom_hi) {
        ps->__axiom_need = 0;
        ps->__axiom_value = 0;
        return -1;
    }
    ps->__axiom_value = (ps->__axiom_value << 6) | (b & 0x3f);
    ps->__axiom_lo = 0x80;
    ps->__axiom_hi = 0xbf;
    if (--ps->__axiom_need > 0)
        return -2;
    {
        long c = (long)ps->__axiom_value;
        ps->__axiom_value = 0;
        return c;
    }
}

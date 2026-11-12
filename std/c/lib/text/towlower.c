/* towlower (C17 7.30.3.1.1): ASCII's mapping in a single-byte locale, and
   Unicode's simple one in a UTF-8 locale (std/c/rt/text.ax). */
#include <axc/text.h>
#include <wctype.h>

wint_t towlower(wint_t wc) {
    unsigned long c = (unsigned long)wc;
    if (c < 0x80)
        return c >= 'A' && c <= 'Z' ? (wint_t)(c + 32) : wc;
    if (c > 0x10ffff || !__axc_mb_utf8())
        return wc;
    return (wint_t)__axc_wlower_unicode(c);
}

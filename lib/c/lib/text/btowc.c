/* btowc (C17 7.29.6.1.1): a byte that is a character in the initial
   shift state by itself: any byte in a single-byte locale, an ASCII one in
   UTF-8. c is read as an unsigned char (EOF aside). */
#include <axc/text.h>
#include <wchar.h>

wint_t btowc(int c) {
    if (c == -1)
        return WEOF;
    c = (unsigned char)c;
    if (__axc_mb_utf8() && c >= 0x80)
        return WEOF;
    return (wint_t)c;
}

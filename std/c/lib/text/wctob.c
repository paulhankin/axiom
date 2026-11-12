/* wctob (C17 7.29.6.1.2): the single-byte character of c, or EOF. */
#include <axc/text.h>
#include <wchar.h>

int wctob(wint_t c) {
    unsigned long v = (unsigned long)c;
    if (c == WEOF)
        return -1;
    if (v < 0x80 || (v <= 0xff && !__axc_mb_utf8()))
        return (int)v;
    return -1;
}

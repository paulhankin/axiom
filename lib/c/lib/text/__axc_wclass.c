/* __axc_wclass - the classes of a wide character in the current locale
   (axc/text.h): ASCII's as <ctype.h> has them in the "C" locale (C17
   7.4.1, 7.30.2.1), and in a UTF-8 locale Unicode's beyond ASCII
   (__axc_wctab.c's tables: two loads). WEOF, and every other value, has
   none. */
#include <axc/text.h>

unsigned int __axc_wclass(unsigned long c) {
    if (c >= 0x80)
        return c <= 0x10ffff && __axc_mb_utf8() ? __AXC_WCT_CLASS(c) : 0;
    if (c < 0x20 || c == 0x7f) {
        if (c == '\t')
            return __AXC_W_CNTRL | __AXC_W_SPACE | __AXC_W_BLANK;
        if (c >= '\n' && c <= '\r')
            return __AXC_W_CNTRL | __AXC_W_SPACE;
        return __AXC_W_CNTRL;
    }
    if (c == ' ')
        return __AXC_W_PRINT | __AXC_W_SPACE | __AXC_W_BLANK;
    if (c >= '0' && c <= '9')
        return __AXC_W_PRINT | __AXC_W_GRAPH | __AXC_W_DIGIT | __AXC_W_XDIGIT | __AXC_W_ALNUM;
    if (c >= 'A' && c <= 'Z')
        return __AXC_W_PRINT | __AXC_W_GRAPH | __AXC_W_UPPER | __AXC_W_ALPHA | __AXC_W_ALNUM |
               (c <= 'F' ? __AXC_W_XDIGIT : 0);
    if (c >= 'a' && c <= 'z')
        return __AXC_W_PRINT | __AXC_W_GRAPH | __AXC_W_LOWER | __AXC_W_ALPHA | __AXC_W_ALNUM |
               (c <= 'f' ? __AXC_W_XDIGIT : 0);
    return __AXC_W_PRINT | __AXC_W_GRAPH | __AXC_W_PUNCT;
}

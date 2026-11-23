/* __axc_mb_put - a character in LC_CTYPE's encoding (axc/text.h). */
#include <axc/text.h>

int __axc_mb_put(char *s, unsigned long c) {
    if (!__axc_mb_utf8()) {
        if (c > 0xff)
            return -1;
        s[0] = (char)c;
        return 1;
    }
    if (c > 0x10ffff || (c >= 0xd800 && c <= 0xdfff))
        return -1;
    return __axc_utf8_put(s, c);
}

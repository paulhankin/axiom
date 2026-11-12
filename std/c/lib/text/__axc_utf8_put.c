/* __axc_utf8_put - the UTF-8 form of a code point (axc/text.h). */
#include <axc/text.h>

int __axc_utf8_put(char *s, unsigned long c) {
    if (c < 0x80) {
        s[0] = (char)c;
        return 1;
    }
    if (c < 0x800) {
        s[0] = (char)(0xc0 | (c >> 6));
        s[1] = (char)(0x80 | (c & 0x3f));
        return 2;
    }
    if (c < 0x10000) {
        s[0] = (char)(0xe0 | (c >> 12));
        s[1] = (char)(0x80 | ((c >> 6) & 0x3f));
        s[2] = (char)(0x80 | (c & 0x3f));
        return 3;
    }
    s[0] = (char)(0xf0 | (c >> 18));
    s[1] = (char)(0x80 | ((c >> 12) & 0x3f));
    s[2] = (char)(0x80 | ((c >> 6) & 0x3f));
    s[3] = (char)(0x80 | (c & 0x3f));
    return 4;
}

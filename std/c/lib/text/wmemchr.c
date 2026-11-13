/* wmemchr (C17 7.29.4.5.8): the first of n wide characters that is c, or
   a null pointer; 16 bytes at a time (std/c/rt/text_wide.ax). */
#include <axc/text.h>
#include <wchar.h>
#include <string.h>

wchar_t *(wmemchr)(const wchar_t *s, wchar_t c, size_t n) {
    size_t i = (size_t)__axc_wfind(s, (unsigned long long)c, n, __AXC_WBITS);
    return i < n ? (wchar_t *)(s + i) : 0;
}

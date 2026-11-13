/* wcslen (C17 7.29.4.6.1): the wide characters before the null one,
   16 bytes at a time (std/c/rt/text_wide.ax). */
#include <axc/text.h>
#include <wchar.h>
#include <string.h>

size_t wcslen(const wchar_t *s) {
    return (size_t)__axc_wlen(s, __AXC_WBITS);
}

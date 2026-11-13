/* wmemcmp (C17 7.29.4.4.5): the order of the first of n wide characters
   that differ, as wchar_t values (16 bytes at a time: text_wide.ax). */
#include <axc/text.h>
#include <wchar.h>
#include <string.h>

int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n) {
    size_t i = (size_t)__axc_wdiff(s1, s2, n, __AXC_WBITS);
    if (i == n)
        return 0;
    return s1[i] < s2[i] ? -1 : 1;
}

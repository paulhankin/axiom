/* wmemchr (C17 7.29.4.5.8). (The name is in parentheses: in C23 <wchar.h>
   makes it a macro.) */
#include <wchar.h>

wchar_t *(wmemchr)(const wchar_t *s, wchar_t c, size_t n) {
    size_t i;
    for (i = 0; i < n; i++)
        if (s[i] == c)
            return (wchar_t *)(s + i);
    return 0;
}

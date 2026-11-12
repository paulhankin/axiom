/* wcsrchr (C17 7.29.4.5.4). (The name is in parentheses: in C23 <wchar.h>
   makes it a macro.) */
#include <wchar.h>

wchar_t *(wcsrchr)(const wchar_t *s, wchar_t c) {
    const wchar_t *last = 0;
    for (;; s++) {
        if (*s == c)
            last = s;
        if (*s == 0)
            return (wchar_t *)last;
    }
}

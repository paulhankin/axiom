/* wcschr (C17 7.29.4.5.1): the null wide character is part of the string.
   (The name is in parentheses: in C23 <wchar.h> makes it a macro.) */
#include <wchar.h>

wchar_t *(wcschr)(const wchar_t *s, wchar_t c) {
    for (;; s++) {
        if (*s == c)
            return (wchar_t *)s;
        if (*s == 0)
            return 0;
    }
}

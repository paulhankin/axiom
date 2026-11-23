/* wcsstr (C17 7.29.4.5.6): an empty s2 is found at s1. (The name is in
   parentheses: in C23 <wchar.h> makes it a macro.) */
#include <wchar.h>

wchar_t *(wcsstr)(const wchar_t *s1, const wchar_t *s2) {
    if (*s2 == 0)
        return (wchar_t *)s1;
    for (; *s1; s1++) {
        size_t i = 0;
        while (s2[i] && s1[i] == s2[i])
            i++;
        if (s2[i] == 0)
            return (wchar_t *)s1;
    }
    return 0;
}

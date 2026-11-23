/* wcspbrk (C17 7.29.4.5.3). (The name is in parentheses: in C23 <wchar.h>
   makes it a macro.) */
#include <wchar.h>

wchar_t *(wcspbrk)(const wchar_t *s1, const wchar_t *s2) {
    for (; *s1; s1++) {
        const wchar_t *p;
        for (p = s2; *p; p++)
            if (*p == *s1)
                return (wchar_t *)s1;
    }
    return 0;
}

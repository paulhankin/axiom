/* __STDC_ISO_10646__ (C17 6.10.8.2): defined where wchar_t is 32 bits (each
   character of ISO/IEC 10646 is the wchar_t of its code point), as the
   date of the Unicode data the library's classes are (15.1: 202309L); not
   where it is 16 bits (Windows). */
#include <stdio.h>
#include <wchar.h>

int main(void) {
#ifdef __STDC_ISO_10646__
    printf("%d\n", (int)sizeof(wchar_t) == 4 && __STDC_ISO_10646__ == 202309L);
#else
    printf("%d\n", (int)sizeof(wchar_t) == 2);
#endif
    return 0;
}
/* expect: 1 */

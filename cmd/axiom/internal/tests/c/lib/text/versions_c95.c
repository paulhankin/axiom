/* In C95, <wchar.h> and <wctype.h> declare Amendment 1's names, none of
   C99's (wcstoll, iswblank, ...) or later: each later name, defined here
   as a macro that would break a declaration, is not used by them. */
/* check-only */
/* flags: --c-std=c95 */
#define wcstoll !
#define wcstoull !
#define wcstof !
#define wcstold !
#define vfwscanf !
#define vswscanf !
#define vwscanf !
#define iswblank !
#define restrict !
#define char16_t !
#define char32_t !
#define mbrtoc16 !
#define timespec !
#define TIME_UTC !
#include <locale.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

int main(void) {
    mbstate_t st = {0};
    wchar_t w[4];
    wint_t c = btowc('a');
    return (int)mbrtowc(w, "a", 1, &st) + iswalpha(c) + (int)wcslen(L"x") + (WEOF != 0) + (WCHAR_MAX > 0) +
           (int)wcstol(L"1", 0, 10) + towupper(c) + (int)wctype("alpha") + mbsinit(&st);
}

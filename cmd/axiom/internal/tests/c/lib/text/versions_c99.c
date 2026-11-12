/* In C99 the headers have C99's names (wcstoll, iswblank, <inttypes.h>)
   and none of C11's (timespec_get, <uchar.h>) or C23's. */
/* check-only */
/* flags: --c-std=c99 */
#define timespec_get !
#define timespec !
#define TIME_UTC !
#define timegm !
#define PRIb64 !
#define SCNb8 !
#include <inttypes.h>
#include <locale.h>
#include <time.h>
#include <wchar.h>
#include <wctype.h>

int main(void) {
    struct lconv *l = localeconv();
    intmax_t m = strtoimax("1", 0, 10) + imaxabs(-1) + imaxdiv(7, 2).rem + wcstoimax(L"2", 0, 10);
    return (int)m + l->int_p_cs_precedes + iswblank(' ') + (int)wcstoll(L"1", 0, 10) + (int)wcstof(L"1", 0) +
           (int)sizeof(PRId64 SCNiPTR PRIXFAST32 SCNuLEAST16);
}

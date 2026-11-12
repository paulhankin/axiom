/* C23's names: timegm, gmtime_r, localtime_r, timespec_getres and the
   time bases, char8_t with mbrtoc8 and c8rtomb, the b conversions of
   <inttypes.h>, the generic search functions of <wchar.h> (whose result
   points to const for a const argument), the __STDC_VERSION_..._H__
   macros; asctime and ctime are deprecated (C4050). */
/* check-only */
/* flags: --c-std=c23 */
#include <inttypes.h>
#include <locale.h>
#include <time.h>
#include <uchar.h>
#include <wchar.h>
#include <wctype.h>

static_assert(__STDC_VERSION_TIME_H__ == 202311L && __STDC_VERSION_WCHAR_H__ == 202311L &&
              __STDC_VERSION_UCHAR_H__ == 202311L && __STDC_VERSION_INTTYPES_H__ == 202311L &&
              __STDC_VERSION_LOCALE_H__ == 202311L && __STDC_VERSION_WCTYPE_H__ == 202311L);
static_assert(TIME_UTC != TIME_MONOTONIC && TIME_MONOTONIC != TIME_ACTIVE);
static_assert(_Generic(wcschr((const wchar_t *)L"x", L'x'), const wchar_t *: 1, default: 0));
static_assert(_Generic(wcschr((wchar_t *)0, L'x'), wchar_t *: 1, default: 0));
static_assert(_Generic(wcsstr((const wchar_t *)L"x", L"x"), const wchar_t *: 1, default: 0));
static_assert(_Generic(wmemchr((const wchar_t *)L"x", L'x', 1), const wchar_t *: 1, default: 0));
static_assert(_Generic(wcspbrk((const wchar_t *)L"x", L"x"), const wchar_t *: 1, default: 0));
static_assert(_Generic(wcsrchr((const wchar_t *)L"x", L'x'), const wchar_t *: 1, default: 0));
static_assert(_Generic((char8_t)0, unsigned char: 1, default: 0));
static_assert(_Generic(u8'a', char8_t: 1, default: 0));

int main(void) {
    struct tm tm = {0}, out;
    struct timespec ts;
    time_t t = timegm(&tm);
    mbstate_t st = {0};
    char8_t c8;
    char b[8];
    return (gmtime_r(&t, &out) != 0) + (localtime_r(&t, &out) != 0) + timespec_getres(&ts, TIME_ACTIVE) +
           (int)mbrtoc8(&c8, "a", 1, &st) + (int)c8rtomb(b, c8, &st) + (int)sizeof(PRIb64 PRIBLEAST8 SCNbFAST16);
}

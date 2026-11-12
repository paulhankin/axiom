/* In C17 the headers have C11's names (timespec_get, <uchar.h>) and none
   of C23's (timegm, mbrtoc8, the b conversions of <inttypes.h>, ...). */
/* check-only */
/* flags: --c-std=c17 */
#define timegm !
#define timespec_getres !
#define gmtime_r !
#define localtime_r !
#define TIME_MONOTONIC !
#define TIME_ACTIVE !
#define char8_t !
#define mbrtoc8 !
#define c8rtomb !
#define PRIb8 !
#define PRIBMAX !
#define SCNbPTR !
#define __STDC_VERSION_TIME_H__ !
#define __STDC_VERSION_WCHAR_H__ !
#define __STDC_VERSION_UCHAR_H__ !
#include <inttypes.h>
#include <locale.h>
#include <time.h>
#include <uchar.h>
#include <wchar.h>
#include <wctype.h>

int main(void) {
    struct timespec ts;
    mbstate_t st = {0};
    char16_t c16;
    char32_t c32 = U'x';
    char b[8];
    return timespec_get(&ts, TIME_UTC) + (int)mbrtoc16(&c16, "a", 1, &st) + (int)c32rtomb(b, c32, &st) +
           (int)ts.tv_nsec;
}

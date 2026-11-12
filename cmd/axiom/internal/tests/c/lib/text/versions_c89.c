/* In C89, <time.h> and <locale.h> declare only C89's names: each later
   one, defined here as a macro that would break a declaration, is not
   used by them (and struct lconv's C99 members have reserved names). */
/* check-only */
/* flags: --c-std=c89 */
#define timespec_get !
#define timespec_getres !
#define timegm !
#define gmtime_r !
#define localtime_r !
#define timespec !
#define tv_sec !
#define tv_nsec !
#define TIME_UTC !
#define TIME_MONOTONIC !
#define TIME_ACTIVE !
#define int_p_cs_precedes !
#define int_n_cs_precedes !
#define int_p_sep_by_space !
#define int_n_sep_by_space !
#define int_p_sign_posn !
#define int_n_sign_posn !
#define restrict !
#define wint_t !
#define wchar_t !
#define mbstate_t !
#include <locale.h>
#include <time.h>

int main(void) {
    struct tm tm;
    time_t t = time(0);
    tm = *gmtime(&t);
    return localeconv()->frac_digits + tm.tm_year + (int)strftime(0, 0, "", &tm) + (int)CLOCKS_PER_SEC;
}

/* axc/text.h - the internal interface of the C library's text part
   (std/c/lib/text/: <locale.h>, <wchar.h>, <wctype.h>, <uchar.h>,
   <time.h>, <inttypes.h> and the multibyte functions of <stdlib.h>), for
   its own sources. Each __axc_ function is in a source of its name, which
   a program gets when it needs it, like any library function. */
#ifndef __AXIOM_AXC_TEXT_H
#define __AXIOM_AXC_TEXT_H

#include <axc/locale.h>
#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_WCHAR_T
#define __AXIOM_WANT_WINT_T
#define __AXIOM_WANT_MBSTATE_T
#include <axc/types.h>

/* what the text part uses of <stdlib.h>, the core part's (the same
   declarations as its header's: this part's sources do not include it) */
void *malloc(size_t __size);
void free(void *__ptr);
char *getenv(const char *__name);
double strtod(const char *restrict __nptr, char **restrict __endptr);
float strtof(const char *restrict __nptr, char **restrict __endptr);
long double strtold(const char *restrict __nptr, char **restrict __endptr);
long strtol(const char *restrict __nptr, char **restrict __endptr, int __base);
long long strtoll(const char *restrict __nptr, char **restrict __endptr, int __base);
unsigned long strtoul(const char *restrict __nptr, char **restrict __endptr, int __base);
unsigned long long strtoull(const char *restrict __nptr, char **restrict __endptr, int __base);
int mblen(const char *__s, size_t __n);
int mbtowc(wchar_t *restrict __pwc, const char *restrict __s, size_t __n);
int wctomb(char *__s, wchar_t __wc);
size_t mbstowcs(wchar_t *restrict __pwcs, const char *restrict __s, size_t __n);
size_t wcstombs(char *restrict __s, const wchar_t *restrict __pwcs, size_t __n);

/* the current locale: the name of each category (index LC_COLLATE ..
   LC_TIME; LC_ALL's composite name is made when asked for) and whether
   LC_CTYPE's encoding is UTF-8 */
#define __AXC_LOCALE_NAME 64
struct __axc_locale {
    char names[6][__AXC_LOCALE_NAME];
    int utf8;
    char all[6 * (__AXC_LOCALE_NAME + 16)];
};
struct __axc_locale *__axc_locale(void);

/* UTF-8 (RFC 3629, and the Unicode Standard's table 3-7 of well-formed
   byte sequences): one byte b into the decoding state ps. The character's
   code point when b completes one, -2 when more bytes are needed, -1 when
   b cannot continue (or begin) a well-formed sequence; ps is then
   initial again. */
long __axc_utf8_feed(mbstate_t *__ps, unsigned char __b);

/* The UTF-8 bytes of the code point c (at most 0x10FFFF, not a surrogate)
   into s: their number (1 to 4). */
int __axc_utf8_put(char *__s, unsigned long __c);

/* The multibyte bytes of the code point c in LC_CTYPE's encoding into s:
   their number, or -1 if the encoding has no such character (a surrogate,
   beyond U+10FFFF, or beyond 0xFF in a single-byte locale). */
int __axc_mb_put(char *__s, unsigned long __c);

/* The next character of s (at most n bytes) in LC_CTYPE's encoding, from
   the state ps: what mbrtowc returns (the bytes that complete it, 0 for
   the null character, (size_t)-2, (size_t)-1 with errno EILSEQ), with
   the character's code point in *c. */
size_t __axc_mb_get(unsigned long *__c, const char *__s, size_t __n, mbstate_t *__ps);

/* wide character classes (bits of __axc_wclass's result, wctype's values) */
#define __AXC_W_ALNUM 0x001
#define __AXC_W_ALPHA 0x002
#define __AXC_W_BLANK 0x004
#define __AXC_W_CNTRL 0x008
#define __AXC_W_DIGIT 0x010
#define __AXC_W_GRAPH 0x020
#define __AXC_W_LOWER 0x040
#define __AXC_W_PRINT 0x080
#define __AXC_W_PUNCT 0x100
#define __AXC_W_SPACE 0x200
#define __AXC_W_UPPER 0x400
#define __AXC_W_XDIGIT 0x800

/* the classes of a wide character in the current locale (ASCII only in a
   single-byte locale; Unicode's properties in a UTF-8 one) */
unsigned int __axc_wclass(unsigned long __c);
/* (the functions of std/c/rt/ take and give 64-bit integers, the only
   width C has with any --c-int and --c-long) */
/* The wide string searches, 16 bytes at a time (std/c/rt/text_wide.ax),
   for elements of width bits: the first of n that is c (or n), a wide
   string's length, the first index below n where two arrays differ (or n) */
unsigned long long __axc_wfind(const void *__p, unsigned long long __c, unsigned long long __n, unsigned long long __w);
unsigned long long __axc_wlen(const void *__p, unsigned long long __w);
unsigned long long __axc_wdiff(const void *__a, const void *__b, unsigned long long __n, unsigned long long __w);
#define __AXC_WBITS ((unsigned long long)(sizeof(wchar_t) * 8))

/* The classes and simple case mappings of a code point c (at most
   U+10FFFF) in a UTF-8 locale: two-stage tables of Unicode's properties
   (__axc_wctab.c, which cmd/internal/tools/gen_c_wctype.py generates from
   std/unicode/tables.ax) */
extern const unsigned short __axc_wct_cval[];
extern const unsigned char __axc_wct_cblk[];
extern const unsigned char __axc_wct_class[];
extern const int __axc_wct_up[];
extern const int __axc_wct_lo[];
extern const unsigned char __axc_wct_kblk[];
extern const unsigned char __axc_wct_case[];
#define __AXC_WCT_CLASS(c) (__axc_wct_cval[__axc_wct_class[__axc_wct_cblk[(c) >> 7] * 128u + ((c) & 127u)]])
#define __AXC_WCT_CASE(c) (__axc_wct_case[__axc_wct_kblk[(c) >> 7] * 128u + ((c) & 127u)])

/* A UTF-16 unit's key in code point order (wcscoll, wcsxfrm where wchar_t
   has 16 bits): U+E000-U+FFFF before the surrogates, which stand for the
   characters beyond U+FFFF; a 16-bit value */
static inline unsigned int __axc_wcs_key(wchar_t u) {
    unsigned int v = (unsigned int)u & 0xFFFFu;
    return v >= 0xE000u ? v - 0x800u : v >= 0xD800u ? v + 0x2000u : v;
}

/* the wide numeric conversions' narrow string (__axc_wcs_narrow.c) */
char *__axc_wcs_narrow(const wchar_t *__s, char *__buf, size_t __cap, size_t *__lead);
void __axc_wcs_narrow_done(char *__p, char *__buf);

/* time (std/c/lib/text/, std/c/rt/text_time.ax) */
struct tm;
/* the runtime's clocks: 0 real time since 1970, 1 monotonic, 2 the
   processor time of the process; in nanoseconds, and their resolution */
long long __axc_clock_ns(unsigned long long __clock);
long long __axc_clock_res(unsigned long long __clock);
/* the local zone at the instant t: offset east of UTC (seconds), whether
   daylight saving time is in force, the abbreviation (cap bytes) */
void __axc_tz_info(long long __t, long long *__off, long long *__dst, char *__abbr, size_t __cap);
/* the instant of a local time l, as mktime reads it with tm_isdst */
long long __axc_tz_from_local(long long __l, long long __isdst);
/* days since 1970-01-01 of a proleptic Gregorian date (m 1 to 12, d 1 to
   31; y within +-2^40) */
long long __axc_days_from_civil(long long __y, int __m, int __d);
/* the broken-down time of s seconds since 1970 of a calendar (UTC, or a
   local time with its offset added): 0, or -1 if its year is not in an
   int with 1900 taken away (*tm unchanged); tm_isdst is not set */
int __axc_tm_from_seconds(struct tm *tm, long long __s);
/* the seconds since 1970 of a broken-down time, its fields normalized
   (C17 7.27.2.3p2: any int values) */
long long __axc_tm_to_seconds(const struct tm *tm);
/* the ISO 8601 week-based year of a broken-down time, and its week (1 to
   53) in *week (C17 7.27.3.5p4) */
long long __axc_iso_week(const struct tm *tm, int *__week);
/* asctime's conversion */
char *__axc_asctime(const struct tm *tm);
/* the same into result (64 bytes), which it returns */
char *__axc_asctime_to(char *__result, const struct tm *tm);
/* the object that gmtime and localtime return */
struct tm *__axc_tm_static(void);
/* strftime's engine: the conversion of one specifier c (with modifier m,
   'E', 'O' or 0) of tm into s (at most n bytes): their number, or -1 if
   c is not a conversion specifier */
int __axc_strftime_one(char *__s, size_t __n, int __c, int __m, const struct tm *tm);

#endif

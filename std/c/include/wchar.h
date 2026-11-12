/* wchar.h - extended multibyte and wide character utilities (C95, C99
   7.24, C11 7.29, C23 7.31), for the Axiom compiler's C library. wchar_t is
   UTF-32 (UTF-16 on Windows, where a character beyond U+FFFF is two
   wchar_t: a surrogate pair); the multibyte encoding is the locale's
   (axc/locale.h). The input/output functions are the stdio part's
   (std/c/lib/stdio/), the others the text part's (std/c/lib/text/). */
#ifndef __AXIOM_WCHAR_H
#define __AXIOM_WCHAR_H

#if __STDC_VERSION__ < 199409L
#error "<wchar.h> is a header of C95 and later"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_WCHAR_H__ 202311L
#endif

#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_WCHAR_T
#define __AXIOM_WANT_WINT_T
#define __AXIOM_WANT_MBSTATE_T
#include <axc/types.h>
#include <axc/wchar_limits.h>

#define NULL ((void *)0)
#define WEOF ((wint_t)-1)

#if __STDC_VERSION__ >= 199901L
#define __AXIOM_WR restrict
#else
#define __AXIOM_WR
#endif

struct tm;
struct __axc_file;

/* formatted wide character input/output (7.29.2) */
int fwprintf(struct __axc_file *__AXIOM_WR stream, const wchar_t *__AXIOM_WR format, ...);
int fwscanf(struct __axc_file *__AXIOM_WR stream, const wchar_t *__AXIOM_WR format, ...);
int swprintf(wchar_t *__AXIOM_WR s, size_t n, const wchar_t *__AXIOM_WR format, ...);
int swscanf(const wchar_t *__AXIOM_WR s, const wchar_t *__AXIOM_WR format, ...);
int vfwprintf(struct __axc_file *__AXIOM_WR stream, const wchar_t *__AXIOM_WR format, __builtin_va_list arg);
int vswprintf(wchar_t *__AXIOM_WR s, size_t n, const wchar_t *__AXIOM_WR format, __builtin_va_list arg);
int vwprintf(const wchar_t *__AXIOM_WR format, __builtin_va_list arg);
int wprintf(const wchar_t *__AXIOM_WR format, ...);
int wscanf(const wchar_t *__AXIOM_WR format, ...);
#if __STDC_VERSION__ >= 199901L
int vfwscanf(struct __axc_file *restrict stream, const wchar_t *restrict format, __builtin_va_list arg);
int vswscanf(const wchar_t *restrict s, const wchar_t *restrict format, __builtin_va_list arg);
int vwscanf(const wchar_t *restrict format, __builtin_va_list arg);
#endif

#if __STDC_VERSION__ >= 202311L
/* C23 mode's formatted wide input: %i takes the 0b prefix */
int __axc_fwscanf_c23(struct __axc_file *restrict, const wchar_t *restrict, ...);
int __axc_swscanf_c23(const wchar_t *restrict, const wchar_t *restrict, ...);
int __axc_wscanf_c23(const wchar_t *restrict, ...);
int __axc_vfwscanf_c23(struct __axc_file *restrict, const wchar_t *restrict, __builtin_va_list);
int __axc_vswscanf_c23(const wchar_t *restrict, const wchar_t *restrict, __builtin_va_list);
int __axc_vwscanf_c23(const wchar_t *restrict, __builtin_va_list);
#define fwscanf(...) __axc_fwscanf_c23(__VA_ARGS__)
#define swscanf(...) __axc_swscanf_c23(__VA_ARGS__)
#define wscanf(...) __axc_wscanf_c23(__VA_ARGS__)
#define vfwscanf(...) __axc_vfwscanf_c23(__VA_ARGS__)
#define vswscanf(...) __axc_vswscanf_c23(__VA_ARGS__)
#define vwscanf(...) __axc_vwscanf_c23(__VA_ARGS__)
#endif

/* wide character input/output (7.29.3) */
wint_t fgetwc(struct __axc_file *stream);
wchar_t *fgetws(wchar_t *__AXIOM_WR s, int n, struct __axc_file *__AXIOM_WR stream);
wint_t fputwc(wchar_t c, struct __axc_file *stream);
int fputws(const wchar_t *__AXIOM_WR s, struct __axc_file *__AXIOM_WR stream);
int fwide(struct __axc_file *stream, int mode);
wint_t getwc(struct __axc_file *stream);
wint_t getwchar(void);
wint_t putwc(wchar_t c, struct __axc_file *stream);
wint_t putwchar(wchar_t c);
wint_t ungetwc(wint_t c, struct __axc_file *stream);

/* numeric conversions (7.29.4.1) */
double wcstod(const wchar_t *__AXIOM_WR nptr, wchar_t **__AXIOM_WR endptr);
long wcstol(const wchar_t *__AXIOM_WR nptr, wchar_t **__AXIOM_WR endptr, int base);
unsigned long wcstoul(const wchar_t *__AXIOM_WR nptr, wchar_t **__AXIOM_WR endptr, int base);
#if __STDC_VERSION__ >= 199901L
float wcstof(const wchar_t *restrict nptr, wchar_t **restrict endptr);
long double wcstold(const wchar_t *restrict nptr, wchar_t **restrict endptr);
long long wcstoll(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);
unsigned long long wcstoull(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);
/* C23's: the binary prefixes too (the names C23 mode maps them to) */
long __axc_wcstol_c23(const wchar_t *__AXIOM_WR, wchar_t **__AXIOM_WR, int);
long long __axc_wcstoll_c23(const wchar_t *__AXIOM_WR, wchar_t **__AXIOM_WR, int);
unsigned long __axc_wcstoul_c23(const wchar_t *__AXIOM_WR, wchar_t **__AXIOM_WR, int);
unsigned long long __axc_wcstoull_c23(const wchar_t *__AXIOM_WR, wchar_t **__AXIOM_WR, int);
#if __STDC_VERSION__ >= 202311L
#define wcstol(nptr, endptr, base) __axc_wcstol_c23(nptr, endptr, base)
#define wcstoll(nptr, endptr, base) __axc_wcstoll_c23(nptr, endptr, base)
#define wcstoul(nptr, endptr, base) __axc_wcstoul_c23(nptr, endptr, base)
#define wcstoull(nptr, endptr, base) __axc_wcstoull_c23(nptr, endptr, base)
#endif
#endif

/* wide string copying, concatenation, comparison, search (7.29.4.2-5) */
wchar_t *wcscpy(wchar_t *__AXIOM_WR s1, const wchar_t *__AXIOM_WR s2);
wchar_t *wcsncpy(wchar_t *__AXIOM_WR s1, const wchar_t *__AXIOM_WR s2, size_t n);
wchar_t *wmemcpy(wchar_t *__AXIOM_WR s1, const wchar_t *__AXIOM_WR s2, size_t n);
wchar_t *wmemmove(wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wcscat(wchar_t *__AXIOM_WR s1, const wchar_t *__AXIOM_WR s2);
wchar_t *wcsncat(wchar_t *__AXIOM_WR s1, const wchar_t *__AXIOM_WR s2, size_t n);
int wcscmp(const wchar_t *s1, const wchar_t *s2);
int wcscoll(const wchar_t *s1, const wchar_t *s2);
int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n);
size_t wcsxfrm(wchar_t *__AXIOM_WR s1, const wchar_t *__AXIOM_WR s2, size_t n);
int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n);
wchar_t *wcschr(const wchar_t *s, wchar_t c);
size_t wcscspn(const wchar_t *s1, const wchar_t *s2);
wchar_t *wcspbrk(const wchar_t *s1, const wchar_t *s2);
wchar_t *wcsrchr(const wchar_t *s, wchar_t c);
size_t wcsspn(const wchar_t *s1, const wchar_t *s2);
wchar_t *wcsstr(const wchar_t *s1, const wchar_t *s2);
wchar_t *wcstok(wchar_t *__AXIOM_WR s1, const wchar_t *__AXIOM_WR s2, wchar_t **__AXIOM_WR ptr);
wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n);
size_t wcslen(const wchar_t *s);
wchar_t *wmemset(wchar_t *s, wchar_t c, size_t n);

/* time conversion (7.29.5) */
size_t wcsftime(wchar_t *__AXIOM_WR s, size_t maxsize, const wchar_t *__AXIOM_WR format,
                const struct tm *__AXIOM_WR timeptr);

/* multibyte and wide character conversion (7.29.6) */
wint_t btowc(int c);
int wctob(wint_t c);
int mbsinit(const mbstate_t *ps);
size_t mbrlen(const char *__AXIOM_WR s, size_t n, mbstate_t *__AXIOM_WR ps);
size_t mbrtowc(wchar_t *__AXIOM_WR pwc, const char *__AXIOM_WR s, size_t n, mbstate_t *__AXIOM_WR ps);
size_t wcrtomb(char *__AXIOM_WR s, wchar_t wc, mbstate_t *__AXIOM_WR ps);
size_t mbsrtowcs(wchar_t *__AXIOM_WR dst, const char **__AXIOM_WR src, size_t len, mbstate_t *__AXIOM_WR ps);
size_t wcsrtombs(char *__AXIOM_WR dst, const wchar_t **__AXIOM_WR src, size_t len, mbstate_t *__AXIOM_WR ps);

#if __STDC_VERSION__ >= 202311L
/* C23 7.31.4.6p1: the search functions are generic: their result points
   to const when their first argument does */
#define wcschr(s, c) _Generic((s), const wchar_t *: (const wchar_t *)wcschr((s), (c)), default: wcschr((s), (c)))
#define wcspbrk(s1, s2) _Generic((s1), const wchar_t *: (const wchar_t *)wcspbrk((s1), (s2)), \
    default: wcspbrk((s1), (s2)))
#define wcsrchr(s, c) _Generic((s), const wchar_t *: (const wchar_t *)wcsrchr((s), (c)), default: wcsrchr((s), (c)))
#define wcsstr(s1, s2) _Generic((s1), const wchar_t *: (const wchar_t *)wcsstr((s1), (s2)), \
    default: wcsstr((s1), (s2)))
#define wmemchr(s, c, n) _Generic((s), const wchar_t *: (const wchar_t *)wmemchr((s), (c), (n)), \
    default: wmemchr((s), (c), (n)))
#endif

#undef __AXIOM_WR

#endif

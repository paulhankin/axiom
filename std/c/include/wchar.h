/* wchar.h - extended multibyte and wide character utilities (C95, C99
   7.24, C11 7.29, C23 7.31), for the Axiom compiler's C library. wchar_t is
   UTF-32 (UTF-16 on Windows, where a character beyond U+FFFF is two
   wchar_t: a surrogate pair); the multibyte encoding is the locale's
   (axc/locale.h). The input/output functions are the stdio part's
   (std/c/lib/stdio/), the others the text part's (std/c/lib/text/). */
/* (Annex K: whether its names are asked for, K.3.1.1) */
#include <axc/ext1.h>
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
int fwprintf(struct __axc_file *__AXIOM_WR __stream, const wchar_t *__AXIOM_WR __format, ...);
int fwscanf(struct __axc_file *__AXIOM_WR __stream, const wchar_t *__AXIOM_WR __format, ...);
int swprintf(wchar_t *__AXIOM_WR __s, size_t __n, const wchar_t *__AXIOM_WR __format, ...);
int swscanf(const wchar_t *__AXIOM_WR __s, const wchar_t *__AXIOM_WR __format, ...);
int vfwprintf(struct __axc_file *__AXIOM_WR __stream, const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int vswprintf(wchar_t *__AXIOM_WR __s, size_t __n, const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int vwprintf(const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int wprintf(const wchar_t *__AXIOM_WR __format, ...);
int wscanf(const wchar_t *__AXIOM_WR __format, ...);
#if __STDC_VERSION__ >= 199901L
int vfwscanf(struct __axc_file *restrict __stream, const wchar_t *restrict __format, __builtin_va_list __arg);
int vswscanf(const wchar_t *restrict __s, const wchar_t *restrict __format, __builtin_va_list __arg);
int vwscanf(const wchar_t *restrict __format, __builtin_va_list __arg);
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
wint_t fgetwc(struct __axc_file *__stream);
wchar_t *fgetws(wchar_t *__AXIOM_WR __s, int __n, struct __axc_file *__AXIOM_WR __stream);
wint_t fputwc(wchar_t __c, struct __axc_file *__stream);
int fputws(const wchar_t *__AXIOM_WR __s, struct __axc_file *__AXIOM_WR __stream);
int fwide(struct __axc_file *__stream, int __mode);
wint_t getwc(struct __axc_file *__stream);
wint_t getwchar(void);
wint_t putwc(wchar_t __c, struct __axc_file *__stream);
wint_t putwchar(wchar_t __c);
wint_t ungetwc(wint_t __c, struct __axc_file *__stream);

/* numeric conversions (7.29.4.1) */
double wcstod(const wchar_t *__AXIOM_WR __nptr, wchar_t **__AXIOM_WR __endptr);
long wcstol(const wchar_t *__AXIOM_WR __nptr, wchar_t **__AXIOM_WR __endptr, int __base);
unsigned long wcstoul(const wchar_t *__AXIOM_WR __nptr, wchar_t **__AXIOM_WR __endptr, int __base);
#if __STDC_VERSION__ >= 199901L
float wcstof(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
long double wcstold(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
#if defined(__STDC_WANT_IEC_60559_TYPES_EXT__) && __STDC_VERSION__ >= 199901L
/* C23 Annex H.12 (std/c/lib/math/fn_str.c) */
_Float16 wcstof16(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
_Float32 wcstof32(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
_Float64 wcstof64(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
_Float128 wcstof128(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
_Float32x wcstof32x(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
_Float64x wcstof64x(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
/* H.12.5: to the interchange formats' encodings (std/c/lib/math/fn_strenc.c) */
#include <axc/wchar_floatn.h>
#if __STDC_VERSION__ >= 202311L && defined(__STDC_IEC_60559_DFP__)
_Decimal64x wcstod64x(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
#endif
#endif
#ifdef __STDC_IEC_60559_DFP__
_Decimal32 wcstod32(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
_Decimal64 wcstod64(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
_Decimal128 wcstod128(const wchar_t *restrict __nptr, wchar_t **restrict __endptr);
#endif
long long wcstoll(const wchar_t *restrict __nptr, wchar_t **restrict __endptr, int __base);
unsigned long long wcstoull(const wchar_t *restrict __nptr, wchar_t **restrict __endptr, int __base);
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
wchar_t *wcscpy(wchar_t *__AXIOM_WR __s1, const wchar_t *__AXIOM_WR __s2);
wchar_t *wcsncpy(wchar_t *__AXIOM_WR __s1, const wchar_t *__AXIOM_WR __s2, size_t __n);
wchar_t *wmemcpy(wchar_t *__AXIOM_WR __s1, const wchar_t *__AXIOM_WR __s2, size_t __n);
wchar_t *wmemmove(wchar_t *__s1, const wchar_t *__s2, size_t __n);
wchar_t *wcscat(wchar_t *__AXIOM_WR __s1, const wchar_t *__AXIOM_WR __s2);
wchar_t *wcsncat(wchar_t *__AXIOM_WR __s1, const wchar_t *__AXIOM_WR __s2, size_t __n);
int wcscmp(const wchar_t *__s1, const wchar_t *__s2);
int wcscoll(const wchar_t *__s1, const wchar_t *__s2);
int wcsncmp(const wchar_t *__s1, const wchar_t *__s2, size_t __n);
size_t wcsxfrm(wchar_t *__AXIOM_WR __s1, const wchar_t *__AXIOM_WR __s2, size_t __n);
int wmemcmp(const wchar_t *__s1, const wchar_t *__s2, size_t __n);
wchar_t *wcschr(const wchar_t *__s, wchar_t __c);
size_t wcscspn(const wchar_t *__s1, const wchar_t *__s2);
wchar_t *wcspbrk(const wchar_t *__s1, const wchar_t *__s2);
wchar_t *wcsrchr(const wchar_t *__s, wchar_t __c);
size_t wcsspn(const wchar_t *__s1, const wchar_t *__s2);
wchar_t *wcsstr(const wchar_t *__s1, const wchar_t *__s2);
wchar_t *wcstok(wchar_t *__AXIOM_WR __s1, const wchar_t *__AXIOM_WR __s2, wchar_t **__AXIOM_WR __ptr);
wchar_t *wmemchr(const wchar_t *__s, wchar_t __c, size_t __n);
size_t wcslen(const wchar_t *__s);
wchar_t *wmemset(wchar_t *__s, wchar_t __c, size_t __n);

/* time conversion (7.29.5) */
size_t wcsftime(wchar_t *__AXIOM_WR __s, size_t __maxsize, const wchar_t *__AXIOM_WR __format,
                const struct tm *__AXIOM_WR __timeptr);

/* multibyte and wide character conversion (7.29.6) */
wint_t btowc(int __c);
int wctob(wint_t __c);
int mbsinit(const mbstate_t *__ps);
size_t mbrlen(const char *__AXIOM_WR __s, size_t __n, mbstate_t *__AXIOM_WR __ps);
size_t mbrtowc(wchar_t *__AXIOM_WR __pwc, const char *__AXIOM_WR __s, size_t __n, mbstate_t *__AXIOM_WR __ps);
size_t wcrtomb(char *__AXIOM_WR __s, wchar_t __wc, mbstate_t *__AXIOM_WR __ps);
size_t mbsrtowcs(wchar_t *__AXIOM_WR __dst, const char **__AXIOM_WR __src, size_t __len, mbstate_t *__AXIOM_WR __ps);
size_t wcsrtombs(char *__AXIOM_WR __dst, const wchar_t **__AXIOM_WR __src, size_t __len, mbstate_t *__AXIOM_WR __ps);

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

#if __AXC_EXT1
/* Annex K (K.3.9): the bounds-checking interfaces (std/c/lib/ext1/) */
#define __AXIOM_WANT_ERRNO_T
#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_RSIZE_T
#include <axc/types.h>
int fwprintf_s(struct __axc_file *__AXIOM_WR __stream, const wchar_t *__AXIOM_WR __format, ...);
int fwscanf_s(struct __axc_file *__AXIOM_WR __stream, const wchar_t *__AXIOM_WR __format, ...);
int snwprintf_s(wchar_t *__AXIOM_WR __s, rsize_t __n, const wchar_t *__AXIOM_WR __format, ...);
int swprintf_s(wchar_t *__AXIOM_WR __s, rsize_t __n, const wchar_t *__AXIOM_WR __format, ...);
int swscanf_s(const wchar_t *__AXIOM_WR __s, const wchar_t *__AXIOM_WR __format, ...);
int vfwprintf_s(struct __axc_file *__AXIOM_WR __stream, const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int vfwscanf_s(struct __axc_file *__AXIOM_WR __stream, const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int vsnwprintf_s(wchar_t *__AXIOM_WR __s, rsize_t __n, const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int vswprintf_s(wchar_t *__AXIOM_WR __s, rsize_t __n, const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int vswscanf_s(const wchar_t *__AXIOM_WR __s, const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int vwprintf_s(const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int vwscanf_s(const wchar_t *__AXIOM_WR __format, __builtin_va_list __arg);
int wprintf_s(const wchar_t *__AXIOM_WR __format, ...);
int wscanf_s(const wchar_t *__AXIOM_WR __format, ...);
errno_t wcscpy_s(wchar_t *__AXIOM_WR __s1, rsize_t __s1max, const wchar_t *__AXIOM_WR __s2);
errno_t wcsncpy_s(wchar_t *__AXIOM_WR __s1, rsize_t __s1max, const wchar_t *__AXIOM_WR __s2, rsize_t __n);
errno_t wmemcpy_s(wchar_t *__AXIOM_WR __s1, rsize_t __s1max, const wchar_t *__AXIOM_WR __s2, rsize_t __n);
errno_t wmemmove_s(wchar_t *__s1, rsize_t __s1max, const wchar_t *__s2, rsize_t __n);
errno_t wcscat_s(wchar_t *__AXIOM_WR __s1, rsize_t __s1max, const wchar_t *__AXIOM_WR __s2);
errno_t wcsncat_s(wchar_t *__AXIOM_WR __s1, rsize_t __s1max, const wchar_t *__AXIOM_WR __s2, rsize_t __n);
wchar_t *wcstok_s(wchar_t *__AXIOM_WR __s1, rsize_t *__AXIOM_WR __s1max, const wchar_t *__AXIOM_WR __s2,
                  wchar_t **__AXIOM_WR __ptr);
size_t wcsnlen_s(const wchar_t *__s, size_t __maxsize);
errno_t wcrtomb_s(size_t *__AXIOM_WR __retval, char *__AXIOM_WR __s, rsize_t __smax, wchar_t __wc, mbstate_t *__AXIOM_WR __ps);
errno_t mbsrtowcs_s(size_t *__AXIOM_WR __retval, wchar_t *__AXIOM_WR __dst, rsize_t __dstmax, const char **__AXIOM_WR __src,
                    rsize_t __len, mbstate_t *__AXIOM_WR __ps);
errno_t wcsrtombs_s(size_t *__AXIOM_WR __retval, char *__AXIOM_WR __dst, rsize_t __dstmax, const wchar_t **__AXIOM_WR __src,
                    rsize_t __len, mbstate_t *__AXIOM_WR __ps);
#if __STDC_VERSION__ >= 202311L
/* (C23 mode: %i takes the 0b prefix, as fwscanf's does) */
int __axc_fwscanf_s_c23(struct __axc_file *restrict, const wchar_t *restrict, ...);
int __axc_swscanf_s_c23(const wchar_t *restrict, const wchar_t *restrict, ...);
int __axc_wscanf_s_c23(const wchar_t *restrict, ...);
int __axc_vfwscanf_s_c23(struct __axc_file *restrict, const wchar_t *restrict, __builtin_va_list);
int __axc_vswscanf_s_c23(const wchar_t *restrict, const wchar_t *restrict, __builtin_va_list);
int __axc_vwscanf_s_c23(const wchar_t *restrict, __builtin_va_list);
#define fwscanf_s(...) __axc_fwscanf_s_c23(__VA_ARGS__)
#define swscanf_s(...) __axc_swscanf_s_c23(__VA_ARGS__)
#define wscanf_s(...) __axc_wscanf_s_c23(__VA_ARGS__)
#define vfwscanf_s(...) __axc_vfwscanf_s_c23(__VA_ARGS__)
#define vswscanf_s(...) __axc_vswscanf_s_c23(__VA_ARGS__)
#define vwscanf_s(...) __axc_vwscanf_s_c23(__VA_ARGS__)
#endif
#endif

#undef __AXIOM_WR

#endif

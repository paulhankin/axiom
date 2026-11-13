/* stdlib.h - general utilities (C89 4.10, C99 7.20, C11 7.22, C23 7.24),
   for the Axiom compiler's C library. The functions are
   std/c/lib/core/<name>.c and std/c/rt/core*.ax (the multibyte ones are
   std/c/lib/text's). Choices the standard leaves to the implementation:

   - malloc(0), calloc with a zero size, aligned_alloc(a, 0) and
     realloc(p, 0) return a unique pointer to an object of no bytes, which
     free takes (realloc(p, 0) gives p's object back first). Memory comes
     from the Axiom allocator (std/mem.ax, thread-safe), or where the
     program has the system's C library (macOS; Linux when a function of it
     is called) from that library's heap, so that the two libraries' memory
     is one (std/c/rt/core_alloc.ax). It is aligned to 16 bytes
     (alignof(max_align_t)) or to an aligned_alloc's alignment, any power of
     two; aligned_alloc fails (null, EINVAL) for another alignment. A failed
     allocation sets errno to ENOMEM.
   - rand: a 64-bit linear congruential generator (multiplier
     6364136223846793005, increment 1442695040888963407), whose high 31 bits
     are the result; RAND_MAX is 2147483647, srand(s) sets the state to s,
     and the first state is 1. One state for all threads (not required to be
     free of data races: 7.22.2.1p3): threads that call rand at once may
     see the same value, or values out of the sequence.
   - qsort: introspective sort (quicksort with the median of three, or of
     nine for large arrays, heapsort past 2 log2(n) partitions, insertion sort
     for short runs): O(n log n) at worst, O(log n) stack. Not stable.
   - strtod and its kin: correctly rounded (decimal and hexadecimal forms);
     a result that underflows (zero, or subnormal, from a nonzero subject)
     sets errno to ERANGE, as overflow does. The decimal point is '.'.
     long double is the C front end's (strtold converts as strtod does).
   - In C23 mode (7.24.1.7) strtol, strtoll, strtoul and strtoull also take
     the 0b and 0B prefixes, for base 0 and 2 (__axc_*_c23, below).
   - exit runs the atexit handlers in the reverse order of their
     registration (any number of them: past 32 the table grows), then
     flushes and closes the streams (through __axc_at_exit_flush, the
     standard I/O's), and ends the process with the status (EXIT_SUCCESS 0,
     EXIT_FAILURE 1). A handler that calls exit makes the remaining handlers
     run once, and the process end with the later status.
   - abort raises SIGABRT; if that returns, the process ends with status 3.
   - getenv's strings stay valid until the same name's value is asked for
     again after it changed. system takes a command for /bin/sh (Linux,
     macOS) or cmd.exe (Windows), whose status it returns (POSIX's encoding
     on Linux and macOS: the exit status times 256, or the signal's number);
     on WebAssembly there is no command processor (system(NULL) is 0, any
     command -1). */
/* (Annex K: whether its names are asked for, K.3.1.1) */
#include <axc/ext1.h>
#ifndef __AXIOM_STDLIB_H
#define __AXIOM_STDLIB_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STDLIB_H__ 202311L
#endif

#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_WCHAR_T
#include <axc/types.h>

#define NULL ((void *)0)

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define RAND_MAX 2147483647

/* the longest multibyte character in the current locale (axc/locale.h:
   the text part's) */
#include <axc/locale.h>
#define MB_CUR_MAX ((size_t)__axc_mb_cur_max())

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

#if __STDC_VERSION__ >= 199901L
typedef struct {
    long long quot;
    long long rem;
} lldiv_t;
#endif

/* numeric conversion (7.24.1) */
double atof(const char *__nptr);
int atoi(const char *__nptr);
long atol(const char *__nptr);
#if __STDC_VERSION__ >= 199901L
long long atoll(const char *__nptr);
#endif
double strtod(const char *__AXC_RESTRICT __nptr, char **__AXC_RESTRICT __endptr);
#if __STDC_VERSION__ >= 199901L
float strtof(const char *restrict __nptr, char **restrict __endptr);
long double strtold(const char *restrict __nptr, char **restrict __endptr);
#endif
long strtol(const char *__AXC_RESTRICT __nptr, char **__AXC_RESTRICT __endptr, int __base);
unsigned long strtoul(const char *__AXC_RESTRICT __nptr, char **__AXC_RESTRICT __endptr, int __base);
#if __STDC_VERSION__ >= 199901L
long long strtoll(const char *restrict __nptr, char **restrict __endptr, int __base);
unsigned long long strtoull(const char *restrict __nptr, char **restrict __endptr, int __base);
#endif
#if __STDC_VERSION__ >= 199901L
/* C23's strtol and its kin, with the binary prefixes (the names C23 mode
   maps them to, below; declared from C99 for the library's C17 units) */
long __axc_strtol_c23(const char *restrict __nptr, char **restrict __endptr, int __base);
long long __axc_strtoll_c23(const char *restrict __nptr, char **restrict __endptr, int __base);
unsigned long __axc_strtoul_c23(const char *restrict __nptr, char **restrict __endptr, int __base);
unsigned long long __axc_strtoull_c23(const char *restrict __nptr, char **restrict __endptr, int __base);
#endif
#if defined(__STDC_WANT_IEC_60559_TYPES_EXT__) && __STDC_VERSION__ >= 199901L
/* ISO/IEC TS 18661-3 (C23 Annex H.12): _Float16's and _Float128's
   conversions are the library's; _Float32's, _Float64's and _Float32x's
   call those of float and double, _Float64x's long double's where it is
   wider than double, else _Float128's (std/c/lib/math/fn_str.c) */
_Float16 strtof16(const char *restrict nptr, char **restrict endptr);
_Float128 strtof128(const char *restrict nptr, char **restrict endptr);
int strfromf16(char *restrict s, size_t n, const char *restrict format, _Float16 fp);
int strfromf128(char *restrict s, size_t n, const char *restrict format, _Float128 fp);
int strfromd(char *restrict s, size_t n, const char *restrict format, double fp);
int strfromf(char *restrict s, size_t n, const char *restrict format, float fp);
int strfroml(char *restrict s, size_t n, const char *restrict format, long double fp);
_Float32 strtof32(const char *restrict nptr, char **restrict endptr);
_Float64 strtof64(const char *restrict nptr, char **restrict endptr);
_Float32x strtof32x(const char *restrict nptr, char **restrict endptr);
_Float64x strtof64x(const char *restrict nptr, char **restrict endptr);
int strfromf32(char *restrict s, size_t n, const char *restrict format, _Float32 fp);
int strfromf64(char *restrict s, size_t n, const char *restrict format, _Float64 fp);
int strfromf32x(char *restrict s, size_t n, const char *restrict format, _Float32x fp);
int strfromf64x(char *restrict s, size_t n, const char *restrict format, _Float64x fp);
/* H.12.4, H.12.5: from and to the interchange formats' encodings
   (std/c/lib/math/fn_strenc.c) */
#include <axc/stdlib_floatn.h>
#if __STDC_VERSION__ >= 202311L && defined(__STDC_IEC_60559_DFP__)
/* _Decimal64x's (H.12.2, H.12.3): _Decimal128's, whose format it has */
_Decimal64x strtod64x(const char *restrict __nptr, char **restrict __endptr);
int strfromd64x(char *restrict __s, size_t __n, const char *restrict __format, _Decimal64x __fp);
#endif
#endif
#if __STDC_VERSION__ >= 202311L
int strfromd(char *restrict __s, size_t __n, const char *restrict __format, double __fp);
int strfromf(char *restrict __s, size_t __n, const char *restrict __format, float __fp);
int strfroml(char *restrict __s, size_t __n, const char *restrict __format, long double __fp);
#ifdef __STDC_IEC_60559_DFP__
/* the decimal floating types (7.24.1.4, 7.24.1.6): correctly rounded in the
   decimal rounding direction, the quantum the number is written with (no
   hexadecimal form); a NaN's n-char-sequence is read and ignored */
int strfromd32(char *restrict __s, size_t __n, const char *restrict __format, _Decimal32 __fp);
int strfromd64(char *restrict __s, size_t __n, const char *restrict __format, _Decimal64 __fp);
int strfromd128(char *restrict __s, size_t __n, const char *restrict __format, _Decimal128 __fp);
_Decimal32 strtod32(const char *restrict __nptr, char **restrict __endptr);
_Decimal64 strtod64(const char *restrict __nptr, char **restrict __endptr);
_Decimal128 strtod128(const char *restrict __nptr, char **restrict __endptr);
#endif
#define strtol(nptr, endptr, base) __axc_strtol_c23(nptr, endptr, base)
#define strtoll(nptr, endptr, base) __axc_strtoll_c23(nptr, endptr, base)
#define strtoul(nptr, endptr, base) __axc_strtoul_c23(nptr, endptr, base)
#define strtoull(nptr, endptr, base) __axc_strtoull_c23(nptr, endptr, base)
#endif

/* pseudo-random sequence generation (7.24.2) */
int rand(void);
void srand(unsigned int __seed);

/* memory management (7.24.3) */
#if __STDC_VERSION__ >= 201112L
void *aligned_alloc(size_t __alignment, size_t __size);
#endif
void *calloc(size_t __nmemb, size_t __size);
void free(void *__ptr);
#if __STDC_VERSION__ >= 202311L
void free_sized(void *__ptr, size_t __size);
void free_aligned_sized(void *__ptr, size_t __alignment, size_t __size);
#endif
void *malloc(size_t __size);
void *realloc(void *__ptr, size_t __size);

/* communication with the environment (7.24.4) */
__AXC_NORETURN void abort(void);
int atexit(void (*__func)(void));
#if __STDC_VERSION__ >= 201112L
int at_quick_exit(void (*__func)(void));
#endif
__AXC_NORETURN void exit(int __status);
#if __STDC_VERSION__ >= 199901L
__AXC_NORETURN void _Exit(int __status);
#endif
char *getenv(const char *__name);
#if __STDC_VERSION__ >= 201112L
__AXC_NORETURN void quick_exit(int __status);
#endif
int system(const char *__string);

/* searching and sorting (7.24.5) */
void *bsearch(const void *__key, const void *__base, size_t __nmemb, size_t __size,
              int (*__compar)(const void *, const void *));
void qsort(void *__base, size_t __nmemb, size_t __size, int (*__compar)(const void *, const void *));
#if __STDC_VERSION__ >= 202311L
/* (7.24.5.1p2: the result points to const when base does; the conditional
   with a void pointer that is not a null pointer constant has the type of
   a pointer to void with base's qualifiers, 6.5.15p6 (6.5.16p6 in C23)) */
#define bsearch(key, base, nmemb, size, compar) \
    _Generic((1 ? (base) : (void *)1), \
             const void *: (const void *)bsearch((key), (base), (nmemb), (size), (compar)), \
             const volatile void *: (const void *)bsearch((key), (base), (nmemb), (size), (compar)), \
             default: bsearch((key), (base), (nmemb), (size), (compar)))
#endif

/* integer arithmetic (7.24.6) */
int abs(int __j);
long labs(long __j);
#if __STDC_VERSION__ >= 199901L
long long llabs(long long __j);
#endif
div_t div(int __numer, int __denom);
ldiv_t ldiv(long __numer, long __denom);
#if __STDC_VERSION__ >= 199901L
lldiv_t lldiv(long long __numer, long long __denom);
#endif

/* multibyte and wide characters (7.24.7, 7.24.8: std/c/lib/text) */
int mblen(const char *__s, size_t __n);
int mbtowc(wchar_t *__AXC_RESTRICT __pwc, const char *__AXC_RESTRICT __s, size_t __n);
int wctomb(char *__s, wchar_t __wc);
size_t mbstowcs(wchar_t *__AXC_RESTRICT __pwcs, const char *__AXC_RESTRICT __s, size_t __n);
size_t wcstombs(char *__AXC_RESTRICT __s, const wchar_t *__AXC_RESTRICT __pwcs, size_t __n);

#if __AXC_EXT1
/* Annex K (K.3.6): the bounds-checking interfaces (std/c/lib/ext1/). The
   runtime-constraint handler that is in force when no
   set_constraint_handler_s has registered one (K.3.6.1.1p3, the
   implementation's choice) is abort_handler_s. */
#define __AXIOM_WANT_ERRNO_T
#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_RSIZE_T
#include <axc/types.h>
typedef void (*constraint_handler_t)(const char *__AXC_RESTRICT __msg, void *__AXC_RESTRICT __ptr, errno_t __error);
constraint_handler_t set_constraint_handler_s(constraint_handler_t __handler);
void abort_handler_s(const char *__AXC_RESTRICT __msg, void *__AXC_RESTRICT __ptr, errno_t __error);
void ignore_handler_s(const char *__AXC_RESTRICT __msg, void *__AXC_RESTRICT __ptr, errno_t __error);
errno_t getenv_s(size_t *__AXC_RESTRICT __len, char *__AXC_RESTRICT __value, rsize_t __maxsize,
                 const char *__AXC_RESTRICT __name);
void *bsearch_s(const void *__key, const void *__base, rsize_t __nmemb, rsize_t __size,
                int (*__compar)(const void *__k, const void *__y, void *__context), void *__context);
errno_t qsort_s(void *__base, rsize_t __nmemb, rsize_t __size, int (*__compar)(const void *__x, const void *__y, void *__context),
                void *__context);
errno_t wctomb_s(int *__AXC_RESTRICT __status, char *__AXC_RESTRICT __s, rsize_t __smax, wchar_t __wc);
errno_t mbstowcs_s(size_t *__AXC_RESTRICT __retval, wchar_t *__AXC_RESTRICT __dst, rsize_t __dstmax,
                   const char *__AXC_RESTRICT __src, rsize_t __len);
errno_t wcstombs_s(size_t *__AXC_RESTRICT __retval, char *__AXC_RESTRICT __dst, rsize_t __dstmax,
                   const wchar_t *__AXC_RESTRICT __src, rsize_t __len);
#endif

#if __STDC_VERSION__ >= 202311L
/* alignment of memory (7.24.9) */
size_t memalignment(const void *__p);
/* communication with the environment: call_once (7.24.2.1; <threads.h>'s) */
#include <axc/once.h>
#endif

#endif

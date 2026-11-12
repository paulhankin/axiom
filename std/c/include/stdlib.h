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
     free of data races: 7.22.2.1p3).
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
double atof(const char *nptr);
int atoi(const char *nptr);
long atol(const char *nptr);
#if __STDC_VERSION__ >= 199901L
long long atoll(const char *nptr);
#endif
double strtod(const char *__AXC_RESTRICT nptr, char **__AXC_RESTRICT endptr);
#if __STDC_VERSION__ >= 199901L
float strtof(const char *restrict nptr, char **restrict endptr);
long double strtold(const char *restrict nptr, char **restrict endptr);
#endif
long strtol(const char *__AXC_RESTRICT nptr, char **__AXC_RESTRICT endptr, int base);
unsigned long strtoul(const char *__AXC_RESTRICT nptr, char **__AXC_RESTRICT endptr, int base);
#if __STDC_VERSION__ >= 199901L
long long strtoll(const char *restrict nptr, char **restrict endptr, int base);
unsigned long long strtoull(const char *restrict nptr, char **restrict endptr, int base);
#endif
#if __STDC_VERSION__ >= 199901L
/* C23's strtol and its kin, with the binary prefixes (the names C23 mode
   maps them to, below; declared from C99 for the library's C17 units) */
long __axc_strtol_c23(const char *restrict nptr, char **restrict endptr, int base);
long long __axc_strtoll_c23(const char *restrict nptr, char **restrict endptr, int base);
unsigned long __axc_strtoul_c23(const char *restrict nptr, char **restrict endptr, int base);
unsigned long long __axc_strtoull_c23(const char *restrict nptr, char **restrict endptr, int base);
#endif
#if __STDC_VERSION__ >= 202311L
int strfromd(char *restrict s, size_t n, const char *restrict format, double fp);
int strfromf(char *restrict s, size_t n, const char *restrict format, float fp);
int strfroml(char *restrict s, size_t n, const char *restrict format, long double fp);
#define strtol(nptr, endptr, base) __axc_strtol_c23(nptr, endptr, base)
#define strtoll(nptr, endptr, base) __axc_strtoll_c23(nptr, endptr, base)
#define strtoul(nptr, endptr, base) __axc_strtoul_c23(nptr, endptr, base)
#define strtoull(nptr, endptr, base) __axc_strtoull_c23(nptr, endptr, base)
#endif

/* pseudo-random sequence generation (7.24.2) */
int rand(void);
void srand(unsigned int seed);

/* memory management (7.24.3) */
#if __STDC_VERSION__ >= 201112L
void *aligned_alloc(size_t alignment, size_t size);
#endif
void *calloc(size_t nmemb, size_t size);
void free(void *ptr);
#if __STDC_VERSION__ >= 202311L
void free_sized(void *ptr, size_t size);
void free_aligned_sized(void *ptr, size_t alignment, size_t size);
#endif
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);

/* communication with the environment (7.24.4) */
__AXC_NORETURN void abort(void);
int atexit(void (*func)(void));
#if __STDC_VERSION__ >= 201112L
int at_quick_exit(void (*func)(void));
#endif
__AXC_NORETURN void exit(int status);
#if __STDC_VERSION__ >= 199901L
__AXC_NORETURN void _Exit(int status);
#endif
char *getenv(const char *name);
#if __STDC_VERSION__ >= 201112L
__AXC_NORETURN void quick_exit(int status);
#endif
int system(const char *string);

/* searching and sorting (7.24.5) */
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
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
int abs(int j);
long labs(long j);
#if __STDC_VERSION__ >= 199901L
long long llabs(long long j);
#endif
div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);
#if __STDC_VERSION__ >= 199901L
lldiv_t lldiv(long long numer, long long denom);
#endif

/* multibyte and wide characters (7.24.7, 7.24.8: std/c/lib/text) */
int mblen(const char *s, size_t n);
int mbtowc(wchar_t *__AXC_RESTRICT pwc, const char *__AXC_RESTRICT s, size_t n);
int wctomb(char *s, wchar_t wc);
size_t mbstowcs(wchar_t *__AXC_RESTRICT pwcs, const char *__AXC_RESTRICT s, size_t n);
size_t wcstombs(char *__AXC_RESTRICT s, const wchar_t *__AXC_RESTRICT pwcs, size_t n);

#if __STDC_VERSION__ >= 202311L
/* alignment of memory (7.24.9) */
size_t memalignment(const void *p);
#endif

#endif

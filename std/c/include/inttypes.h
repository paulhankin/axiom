/* inttypes.h - format conversion of integer types (C99 7.8, C11 7.8, C23
   7.8), for the Axiom compiler's C library. The fprintf and fscanf macros'
   length modifiers are those of the types <stdint.h> chose for the
   target: where the compiler names a type (__AXIOM_INT64_TYPE__ and the
   others), its modifier is read from that type's name, so that the two
   cannot disagree. */
#ifndef __AXIOM_INTTYPES_H
#define __AXIOM_INTTYPES_H

#if __STDC_VERSION__ < 199901L
#error "<inttypes.h> is a header of C99 and later"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_INTTYPES_H__ 202311L
#endif

#include <stdint.h>

/* The length modifier of a type from its name, __AXIOM_LM(type): the
   name's first token, pasted to __AXIOM_LM_, is a macro that takes the
   rest of the name up to the ')' that follows it ("long long", "signed
   char", ...) as its argument. */
#define __AXIOM_LM(T) __AXIOM_LM_X(T)
#define __AXIOM_LM_X(T) __AXIOM_LM_##T )
#define __AXIOM_LM_signed __AXIOM_LM_HH(
#define __AXIOM_LM_HH(rest) "hh"
#define __AXIOM_LM_short __AXIOM_LM_H(
#define __AXIOM_LM_H(rest) "h"
#define __AXIOM_LM_int __AXIOM_LM_NONE(
#define __AXIOM_LM_NONE() ""
#define __AXIOM_LM_long __AXIOM_LM_L(
#define __AXIOM_LM_L(rest) __AXIOM_LM_L_##rest
#define __AXIOM_LM_L_ "l"
#define __AXIOM_LM_L_long "ll"
#define __AXIOM_LM___axiom_int64 __AXIOM_LM_LL(
#define __AXIOM_LM_LL() "ll"

/* the modifiers of int8_t to int64_t (<stdint.h>: signed char, short, int
   or long, long or long long) and of the others */
#define __AXIOM_PRI8 "hh"
#define __AXIOM_PRI16 "h"
#if __AXIOM_INT_WIDTH__ == 32
#define __AXIOM_PRI32 ""
#define __AXIOM_PRILEAST32 ""
#elif __AXIOM_LONG_WIDTH__ == 32
#define __AXIOM_PRI32 "l"
#define __AXIOM_PRILEAST32 "l"
#elif __AXIOM_INT_WIDTH__ == 64
#define __AXIOM_PRILEAST32 ""
#else
#define __AXIOM_PRILEAST32 "l"
#endif
#ifdef __AXIOM_INT64_TYPE__
#define __AXIOM_PRI64 __AXIOM_LM(__AXIOM_INT64_TYPE__)
#elif __AXIOM_LONG_WIDTH__ == 64
#define __AXIOM_PRI64 "l"
#else
#define __AXIOM_PRI64 "ll"
#endif
#ifdef __AXIOM_INT_FAST16_TYPE__
#define __AXIOM_PRIFAST16 __AXIOM_LM(__AXIOM_INT_FAST16_TYPE__)
#else
#define __AXIOM_PRIFAST16 "h"
#endif
#ifdef __AXIOM_INT_FAST32_TYPE__
#define __AXIOM_PRIFAST32 __AXIOM_LM(__AXIOM_INT_FAST32_TYPE__)
#else
#define __AXIOM_PRIFAST32 __AXIOM_PRILEAST32
#endif
#ifdef __AXIOM_INTMAX_TYPE__
#define __AXIOM_PRIMAX __AXIOM_LM(__AXIOM_INTMAX_TYPE__)
#else
#define __AXIOM_PRIMAX __AXIOM_PRI64
#endif
#ifdef __AXIOM_PTRDIFF_TYPE__
#define __AXIOM_PRIPTR __AXIOM_LM(__AXIOM_PTRDIFF_TYPE__)
#elif __AXIOM_INT_WIDTH__ >= __AXIOM_PTR_WIDTH__
#define __AXIOM_PRIPTR ""
#elif __AXIOM_LONG_WIDTH__ >= __AXIOM_PTR_WIDTH__
#define __AXIOM_PRIPTR "l"
#else
#define __AXIOM_PRIPTR "ll"
#endif

/* fprintf (7.8.1p2; in C23 also b and B) and fscanf (7.8.1p3; in C23 also b) */
#define PRId8 __AXIOM_PRI8 "d"
#define PRIi8 __AXIOM_PRI8 "i"
#define PRIo8 __AXIOM_PRI8 "o"
#define PRIu8 __AXIOM_PRI8 "u"
#define PRIx8 __AXIOM_PRI8 "x"
#define PRIX8 __AXIOM_PRI8 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIb8 __AXIOM_PRI8 "b"
#define PRIB8 __AXIOM_PRI8 "B"
#define SCNb8 __AXIOM_PRI8 "b"
#endif
#define SCNd8 __AXIOM_PRI8 "d"
#define SCNi8 __AXIOM_PRI8 "i"
#define SCNo8 __AXIOM_PRI8 "o"
#define SCNu8 __AXIOM_PRI8 "u"
#define SCNx8 __AXIOM_PRI8 "x"
#define PRIdLEAST8 __AXIOM_PRI8 "d"
#define PRIiLEAST8 __AXIOM_PRI8 "i"
#define PRIoLEAST8 __AXIOM_PRI8 "o"
#define PRIuLEAST8 __AXIOM_PRI8 "u"
#define PRIxLEAST8 __AXIOM_PRI8 "x"
#define PRIXLEAST8 __AXIOM_PRI8 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbLEAST8 __AXIOM_PRI8 "b"
#define PRIBLEAST8 __AXIOM_PRI8 "B"
#define SCNbLEAST8 __AXIOM_PRI8 "b"
#endif
#define SCNdLEAST8 __AXIOM_PRI8 "d"
#define SCNiLEAST8 __AXIOM_PRI8 "i"
#define SCNoLEAST8 __AXIOM_PRI8 "o"
#define SCNuLEAST8 __AXIOM_PRI8 "u"
#define SCNxLEAST8 __AXIOM_PRI8 "x"
#define PRIdFAST8 __AXIOM_PRI8 "d"
#define PRIiFAST8 __AXIOM_PRI8 "i"
#define PRIoFAST8 __AXIOM_PRI8 "o"
#define PRIuFAST8 __AXIOM_PRI8 "u"
#define PRIxFAST8 __AXIOM_PRI8 "x"
#define PRIXFAST8 __AXIOM_PRI8 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbFAST8 __AXIOM_PRI8 "b"
#define PRIBFAST8 __AXIOM_PRI8 "B"
#define SCNbFAST8 __AXIOM_PRI8 "b"
#endif
#define SCNdFAST8 __AXIOM_PRI8 "d"
#define SCNiFAST8 __AXIOM_PRI8 "i"
#define SCNoFAST8 __AXIOM_PRI8 "o"
#define SCNuFAST8 __AXIOM_PRI8 "u"
#define SCNxFAST8 __AXIOM_PRI8 "x"
#define PRId16 __AXIOM_PRI16 "d"
#define PRIi16 __AXIOM_PRI16 "i"
#define PRIo16 __AXIOM_PRI16 "o"
#define PRIu16 __AXIOM_PRI16 "u"
#define PRIx16 __AXIOM_PRI16 "x"
#define PRIX16 __AXIOM_PRI16 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIb16 __AXIOM_PRI16 "b"
#define PRIB16 __AXIOM_PRI16 "B"
#define SCNb16 __AXIOM_PRI16 "b"
#endif
#define SCNd16 __AXIOM_PRI16 "d"
#define SCNi16 __AXIOM_PRI16 "i"
#define SCNo16 __AXIOM_PRI16 "o"
#define SCNu16 __AXIOM_PRI16 "u"
#define SCNx16 __AXIOM_PRI16 "x"
#define PRIdLEAST16 __AXIOM_PRI16 "d"
#define PRIiLEAST16 __AXIOM_PRI16 "i"
#define PRIoLEAST16 __AXIOM_PRI16 "o"
#define PRIuLEAST16 __AXIOM_PRI16 "u"
#define PRIxLEAST16 __AXIOM_PRI16 "x"
#define PRIXLEAST16 __AXIOM_PRI16 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbLEAST16 __AXIOM_PRI16 "b"
#define PRIBLEAST16 __AXIOM_PRI16 "B"
#define SCNbLEAST16 __AXIOM_PRI16 "b"
#endif
#define SCNdLEAST16 __AXIOM_PRI16 "d"
#define SCNiLEAST16 __AXIOM_PRI16 "i"
#define SCNoLEAST16 __AXIOM_PRI16 "o"
#define SCNuLEAST16 __AXIOM_PRI16 "u"
#define SCNxLEAST16 __AXIOM_PRI16 "x"
#define PRIdFAST16 __AXIOM_PRIFAST16 "d"
#define PRIiFAST16 __AXIOM_PRIFAST16 "i"
#define PRIoFAST16 __AXIOM_PRIFAST16 "o"
#define PRIuFAST16 __AXIOM_PRIFAST16 "u"
#define PRIxFAST16 __AXIOM_PRIFAST16 "x"
#define PRIXFAST16 __AXIOM_PRIFAST16 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbFAST16 __AXIOM_PRIFAST16 "b"
#define PRIBFAST16 __AXIOM_PRIFAST16 "B"
#define SCNbFAST16 __AXIOM_PRIFAST16 "b"
#endif
#define SCNdFAST16 __AXIOM_PRIFAST16 "d"
#define SCNiFAST16 __AXIOM_PRIFAST16 "i"
#define SCNoFAST16 __AXIOM_PRIFAST16 "o"
#define SCNuFAST16 __AXIOM_PRIFAST16 "u"
#define SCNxFAST16 __AXIOM_PRIFAST16 "x"
#ifdef __AXIOM_HAS_INT32
#define PRId32 __AXIOM_PRI32 "d"
#define PRIi32 __AXIOM_PRI32 "i"
#define PRIo32 __AXIOM_PRI32 "o"
#define PRIu32 __AXIOM_PRI32 "u"
#define PRIx32 __AXIOM_PRI32 "x"
#define PRIX32 __AXIOM_PRI32 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIb32 __AXIOM_PRI32 "b"
#define PRIB32 __AXIOM_PRI32 "B"
#define SCNb32 __AXIOM_PRI32 "b"
#endif
#define SCNd32 __AXIOM_PRI32 "d"
#define SCNi32 __AXIOM_PRI32 "i"
#define SCNo32 __AXIOM_PRI32 "o"
#define SCNu32 __AXIOM_PRI32 "u"
#define SCNx32 __AXIOM_PRI32 "x"
#endif
#define PRIdLEAST32 __AXIOM_PRILEAST32 "d"
#define PRIiLEAST32 __AXIOM_PRILEAST32 "i"
#define PRIoLEAST32 __AXIOM_PRILEAST32 "o"
#define PRIuLEAST32 __AXIOM_PRILEAST32 "u"
#define PRIxLEAST32 __AXIOM_PRILEAST32 "x"
#define PRIXLEAST32 __AXIOM_PRILEAST32 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbLEAST32 __AXIOM_PRILEAST32 "b"
#define PRIBLEAST32 __AXIOM_PRILEAST32 "B"
#define SCNbLEAST32 __AXIOM_PRILEAST32 "b"
#endif
#define SCNdLEAST32 __AXIOM_PRILEAST32 "d"
#define SCNiLEAST32 __AXIOM_PRILEAST32 "i"
#define SCNoLEAST32 __AXIOM_PRILEAST32 "o"
#define SCNuLEAST32 __AXIOM_PRILEAST32 "u"
#define SCNxLEAST32 __AXIOM_PRILEAST32 "x"
#define PRIdFAST32 __AXIOM_PRIFAST32 "d"
#define PRIiFAST32 __AXIOM_PRIFAST32 "i"
#define PRIoFAST32 __AXIOM_PRIFAST32 "o"
#define PRIuFAST32 __AXIOM_PRIFAST32 "u"
#define PRIxFAST32 __AXIOM_PRIFAST32 "x"
#define PRIXFAST32 __AXIOM_PRIFAST32 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbFAST32 __AXIOM_PRIFAST32 "b"
#define PRIBFAST32 __AXIOM_PRIFAST32 "B"
#define SCNbFAST32 __AXIOM_PRIFAST32 "b"
#endif
#define SCNdFAST32 __AXIOM_PRIFAST32 "d"
#define SCNiFAST32 __AXIOM_PRIFAST32 "i"
#define SCNoFAST32 __AXIOM_PRIFAST32 "o"
#define SCNuFAST32 __AXIOM_PRIFAST32 "u"
#define SCNxFAST32 __AXIOM_PRIFAST32 "x"
#define PRId64 __AXIOM_PRI64 "d"
#define PRIi64 __AXIOM_PRI64 "i"
#define PRIo64 __AXIOM_PRI64 "o"
#define PRIu64 __AXIOM_PRI64 "u"
#define PRIx64 __AXIOM_PRI64 "x"
#define PRIX64 __AXIOM_PRI64 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIb64 __AXIOM_PRI64 "b"
#define PRIB64 __AXIOM_PRI64 "B"
#define SCNb64 __AXIOM_PRI64 "b"
#endif
#define SCNd64 __AXIOM_PRI64 "d"
#define SCNi64 __AXIOM_PRI64 "i"
#define SCNo64 __AXIOM_PRI64 "o"
#define SCNu64 __AXIOM_PRI64 "u"
#define SCNx64 __AXIOM_PRI64 "x"
#define PRIdLEAST64 __AXIOM_PRI64 "d"
#define PRIiLEAST64 __AXIOM_PRI64 "i"
#define PRIoLEAST64 __AXIOM_PRI64 "o"
#define PRIuLEAST64 __AXIOM_PRI64 "u"
#define PRIxLEAST64 __AXIOM_PRI64 "x"
#define PRIXLEAST64 __AXIOM_PRI64 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbLEAST64 __AXIOM_PRI64 "b"
#define PRIBLEAST64 __AXIOM_PRI64 "B"
#define SCNbLEAST64 __AXIOM_PRI64 "b"
#endif
#define SCNdLEAST64 __AXIOM_PRI64 "d"
#define SCNiLEAST64 __AXIOM_PRI64 "i"
#define SCNoLEAST64 __AXIOM_PRI64 "o"
#define SCNuLEAST64 __AXIOM_PRI64 "u"
#define SCNxLEAST64 __AXIOM_PRI64 "x"
#define PRIdFAST64 __AXIOM_PRI64 "d"
#define PRIiFAST64 __AXIOM_PRI64 "i"
#define PRIoFAST64 __AXIOM_PRI64 "o"
#define PRIuFAST64 __AXIOM_PRI64 "u"
#define PRIxFAST64 __AXIOM_PRI64 "x"
#define PRIXFAST64 __AXIOM_PRI64 "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbFAST64 __AXIOM_PRI64 "b"
#define PRIBFAST64 __AXIOM_PRI64 "B"
#define SCNbFAST64 __AXIOM_PRI64 "b"
#endif
#define SCNdFAST64 __AXIOM_PRI64 "d"
#define SCNiFAST64 __AXIOM_PRI64 "i"
#define SCNoFAST64 __AXIOM_PRI64 "o"
#define SCNuFAST64 __AXIOM_PRI64 "u"
#define SCNxFAST64 __AXIOM_PRI64 "x"
#define PRIdMAX __AXIOM_PRIMAX "d"
#define PRIiMAX __AXIOM_PRIMAX "i"
#define PRIoMAX __AXIOM_PRIMAX "o"
#define PRIuMAX __AXIOM_PRIMAX "u"
#define PRIxMAX __AXIOM_PRIMAX "x"
#define PRIXMAX __AXIOM_PRIMAX "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbMAX __AXIOM_PRIMAX "b"
#define PRIBMAX __AXIOM_PRIMAX "B"
#define SCNbMAX __AXIOM_PRIMAX "b"
#endif
#define SCNdMAX __AXIOM_PRIMAX "d"
#define SCNiMAX __AXIOM_PRIMAX "i"
#define SCNoMAX __AXIOM_PRIMAX "o"
#define SCNuMAX __AXIOM_PRIMAX "u"
#define SCNxMAX __AXIOM_PRIMAX "x"
#define PRIdPTR __AXIOM_PRIPTR "d"
#define PRIiPTR __AXIOM_PRIPTR "i"
#define PRIoPTR __AXIOM_PRIPTR "o"
#define PRIuPTR __AXIOM_PRIPTR "u"
#define PRIxPTR __AXIOM_PRIPTR "x"
#define PRIXPTR __AXIOM_PRIPTR "X"
#if __STDC_VERSION__ >= 202311L
#define PRIbPTR __AXIOM_PRIPTR "b"
#define PRIBPTR __AXIOM_PRIPTR "B"
#define SCNbPTR __AXIOM_PRIPTR "b"
#endif
#define SCNdPTR __AXIOM_PRIPTR "d"
#define SCNiPTR __AXIOM_PRIPTR "i"
#define SCNoPTR __AXIOM_PRIPTR "o"
#define SCNuPTR __AXIOM_PRIPTR "u"
#define SCNxPTR __AXIOM_PRIPTR "x"

typedef struct {
    intmax_t quot;
    intmax_t rem;
} imaxdiv_t;

#define __AXIOM_WANT_WCHAR_T
#include <axc/types.h>

intmax_t imaxabs(intmax_t j);
imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom);
intmax_t strtoimax(const char *restrict nptr, char **restrict endptr, int base);
uintmax_t strtoumax(const char *restrict nptr, char **restrict endptr, int base);
intmax_t wcstoimax(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);
uintmax_t wcstoumax(const wchar_t *restrict nptr, wchar_t **restrict endptr, int base);
/* C23's: the binary prefixes too (the names C23 mode maps them to) */
intmax_t __axc_strtoimax_c23(const char *restrict, char **restrict, int);
uintmax_t __axc_strtoumax_c23(const char *restrict, char **restrict, int);
intmax_t __axc_wcstoimax_c23(const wchar_t *restrict, wchar_t **restrict, int);
uintmax_t __axc_wcstoumax_c23(const wchar_t *restrict, wchar_t **restrict, int);
#if __STDC_VERSION__ >= 202311L
#define strtoimax(nptr, endptr, base) __axc_strtoimax_c23(nptr, endptr, base)
#define strtoumax(nptr, endptr, base) __axc_strtoumax_c23(nptr, endptr, base)
#define wcstoimax(nptr, endptr, base) __axc_wcstoimax_c23(nptr, endptr, base)
#define wcstoumax(nptr, endptr, base) __axc_wcstoumax_c23(nptr, endptr, base)
#endif

#endif

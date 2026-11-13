/* uchar.h - Unicode utilities (C11 7.28, C23 7.30), for the Axiom
   compiler's C library. char16_t and char32_t are UTF-16 and UTF-32
   (__STDC_UTF_16__, __STDC_UTF_32__), char8_t (C23) UTF-8, in every
   locale; the multibyte side is the locale's encoding (axc/locale.h), in
   which a single-byte locale's byte b is the character U+00bb. */
#ifndef __AXIOM_UCHAR_H
#define __AXIOM_UCHAR_H

#if __STDC_VERSION__ < 201112L
#error "<uchar.h> is a header of C11 and later"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_UCHAR_H__ 202311L
#endif

#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_MBSTATE_T
#include <axc/types.h>

/* (as uint_least16_t and uint_least32_t: the types of u"" and U""
   elements) */
typedef unsigned short char16_t;
#if __AXIOM_INT_WIDTH__ >= 32
typedef unsigned int char32_t;
#else
typedef unsigned long char32_t;
#endif

#if __STDC_VERSION__ >= 202311L
/* (the type of u8"" elements in C23) */
typedef unsigned char char8_t;
size_t mbrtoc8(char8_t *restrict __pc8, const char *restrict __s, size_t __n, mbstate_t *restrict __ps);
size_t c8rtomb(char *restrict __s, char8_t __c8, mbstate_t *restrict __ps);
#endif

size_t mbrtoc16(char16_t *restrict __pc16, const char *restrict __s, size_t __n, mbstate_t *restrict __ps);
size_t c16rtomb(char *restrict __s, char16_t __c16, mbstate_t *restrict __ps);
size_t mbrtoc32(char32_t *restrict __pc32, const char *restrict __s, size_t __n, mbstate_t *restrict __ps);
size_t c32rtomb(char *restrict __s, char32_t __c32, mbstate_t *restrict __ps);

#endif

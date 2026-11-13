/* string.h - string handling (C89 4.11, C99 7.21, C11 7.24, C23 7.26), for
   the Axiom compiler's C front end: the functions C23 (4p7) asks of a
   freestanding implementation, which are all but strdup, strndup, strcoll,
   strxfrm and strerror (those need allocation or a locale). Their
   definitions are std/c/lib/string/<name>.c, which a program gets when it
   uses them and does not define them itself. A hosted program also has the
   others (axc/string_hosted.h: std/c/lib/core/). */
/* (Annex K: whether its names are asked for, K.3.1.1) */
#include <axc/ext1.h>
#ifndef __AXIOM_STRING_H
#define __AXIOM_STRING_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STRING_H__ 202311L
#endif

#define __AXIOM_WANT_SIZE_T
#include <axc/types.h>

/* (as <stddef.h> has it: the same definition again is no redefinition) */
#define NULL ((void *)0)

#if __STDC_VERSION__ >= 199901L
#define __AXIOM_RESTRICT restrict
#else
#define __AXIOM_RESTRICT
#endif

/* copying (7.26.2) */
void *memcpy(void *__AXIOM_RESTRICT __s1, const void *__AXIOM_RESTRICT __s2, size_t __n);
void *memmove(void *__s1, const void *__s2, size_t __n);
char *strcpy(char *__AXIOM_RESTRICT __s1, const char *__AXIOM_RESTRICT __s2);
char *strncpy(char *__AXIOM_RESTRICT __s1, const char *__AXIOM_RESTRICT __s2, size_t __n);
#if __STDC_VERSION__ >= 202311L
void *memccpy(void *restrict __s1, const void *restrict __s2, int __c, size_t __n);
#endif

/* concatenation (7.26.3) */
char *strcat(char *__AXIOM_RESTRICT __s1, const char *__AXIOM_RESTRICT __s2);
char *strncat(char *__AXIOM_RESTRICT __s1, const char *__AXIOM_RESTRICT __s2, size_t __n);

/* comparison (7.26.4) */
int memcmp(const void *__s1, const void *__s2, size_t __n);
int strcmp(const char *__s1, const char *__s2);
int strncmp(const char *__s1, const char *__s2, size_t __n);

/* search (7.26.5) */
void *memchr(const void *__s, int __c, size_t __n);
char *strchr(const char *__s, int __c);
size_t strcspn(const char *__s1, const char *__s2);
char *strpbrk(const char *__s1, const char *__s2);
char *strrchr(const char *__s, int __c);
size_t strspn(const char *__s1, const char *__s2);
char *strstr(const char *__s1, const char *__s2);
char *strtok(char *__AXIOM_RESTRICT __s1, const char *__AXIOM_RESTRICT __s2);

/* miscellaneous (7.26.6) */
void *memset(void *__s, int __c, size_t __n);
size_t strlen(const char *__s);
#if __STDC_VERSION__ >= 202311L
void *memset_explicit(void *__s, int __c, size_t __n);
#endif

#if __STDC_VERSION__ >= 202311L
/* C23 7.26.5p1: the search functions are generic: their result points to
   const when their first argument does */
#define memchr(s, c, n) _Generic((s), const void *: (const void *)memchr((s), (c), (n)), \
    const char *: (const void *)memchr((s), (c), (n)), \
    const unsigned char *: (const void *)memchr((s), (c), (n)), \
    const signed char *: (const void *)memchr((s), (c), (n)), \
    default: memchr((s), (c), (n)))
#define strchr(s, c) _Generic((s), const char *: (const char *)strchr((s), (c)), default: strchr((s), (c)))
#define strpbrk(s1, s2) _Generic((s1), const char *: (const char *)strpbrk((s1), (s2)), \
    default: strpbrk((s1), (s2)))
#define strrchr(s, c) _Generic((s), const char *: (const char *)strrchr((s), (c)), default: strrchr((s), (c)))
#define strstr(s1, s2) _Generic((s1), const char *: (const char *)strstr((s1), (s2)), \
    default: strstr((s1), (s2)))
#endif

#if __AXIOM_HOSTED__
#include <axc/string_hosted.h>
#endif

#endif

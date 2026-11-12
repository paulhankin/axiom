/* string.h - string handling (C89 4.11, C99 7.21, C11 7.24, C23 7.26), for
   the Axiom compiler's C front end: the functions C23 (4p7) asks of a
   freestanding implementation, which are all but strdup, strndup, strcoll,
   strxfrm and strerror (those need allocation or a locale). Their
   definitions are std/c/lib/string/<name>.c, which a program gets when it
   uses them and does not define them itself. */
#ifndef __AXIOM_STRING_H
#define __AXIOM_STRING_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STRING_H__ 202311L
#endif

#ifndef __AXIOM_SIZE_T_DEFINED
#define __AXIOM_SIZE_T_DEFINED
typedef __AXIOM_SIZE_TYPE__ size_t;
#endif

/* (as <stddef.h> has it: the same definition again is no redefinition) */
#define NULL ((void *)0)

#if __STDC_VERSION__ >= 199901L
#define __AXIOM_RESTRICT restrict
#else
#define __AXIOM_RESTRICT
#endif

/* copying (7.26.2) */
void *memcpy(void *__AXIOM_RESTRICT s1, const void *__AXIOM_RESTRICT s2, size_t n);
void *memmove(void *s1, const void *s2, size_t n);
char *strcpy(char *__AXIOM_RESTRICT s1, const char *__AXIOM_RESTRICT s2);
char *strncpy(char *__AXIOM_RESTRICT s1, const char *__AXIOM_RESTRICT s2, size_t n);
#if __STDC_VERSION__ >= 202311L
void *memccpy(void *restrict s1, const void *restrict s2, int c, size_t n);
#endif

/* concatenation (7.26.3) */
char *strcat(char *__AXIOM_RESTRICT s1, const char *__AXIOM_RESTRICT s2);
char *strncat(char *__AXIOM_RESTRICT s1, const char *__AXIOM_RESTRICT s2, size_t n);

/* comparison (7.26.4) */
int memcmp(const void *s1, const void *s2, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);

/* search (7.26.5) */
void *memchr(const void *s, int c, size_t n);
char *strchr(const char *s, int c);
size_t strcspn(const char *s1, const char *s2);
char *strpbrk(const char *s1, const char *s2);
char *strrchr(const char *s, int c);
size_t strspn(const char *s1, const char *s2);
char *strstr(const char *s1, const char *s2);
char *strtok(char *__AXIOM_RESTRICT s1, const char *__AXIOM_RESTRICT s2);

/* miscellaneous (7.26.6) */
void *memset(void *s, int c, size_t n);
size_t strlen(const char *s);
#if __STDC_VERSION__ >= 202311L
void *memset_explicit(void *s, int c, size_t n);
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

#endif

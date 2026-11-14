/* axc/string_hosted.h - the functions of <string.h> (C23 7.26) that C23
   (4p7) does not ask of a freestanding implementation: they need a locale,
   allocation or the library's messages (std/c/lib/core/<name>.c). Included
   by <string.h> in hosted programs. strcoll and strxfrm order strings by
   their bytes in every locale (the "C" locale's order, and the order of
   code points in UTF-8). strerror's messages are the library's own. */
#ifndef __AXIOM_AXC_STRING_HOSTED_H
#define __AXIOM_AXC_STRING_HOSTED_H

#define __AXIOM_WANT_SIZE_T
#include <axc/types.h>

int strcoll(const char *__s1, const char *__s2);
size_t strxfrm(char *__AXC_RESTRICT __s1, const char *__AXC_RESTRICT __s2, size_t __n);
char *strerror(int __errnum);
#if __STDC_VERSION__ >= 202311L || defined(__AXIOM_GNU__)
/* (C23's; in a GNU dialect in every standard, as POSIX has them) */
char *strdup(const char *__s);
char *strndup(const char *__s, size_t __n);
#endif

#if __AXC_EXT1
/* Annex K (K.3.7): the bounds-checking interfaces (std/c/lib/ext1/) */
#define __AXIOM_WANT_ERRNO_T
#define __AXIOM_WANT_SIZE_T
#define __AXIOM_WANT_RSIZE_T
#include <axc/types.h>
errno_t memcpy_s(void *__AXC_RESTRICT __s1, rsize_t __s1max, const void *__AXC_RESTRICT __s2, rsize_t __n);
errno_t memmove_s(void *__s1, rsize_t __s1max, const void *__s2, rsize_t __n);
errno_t strcpy_s(char *__AXC_RESTRICT __s1, rsize_t __s1max, const char *__AXC_RESTRICT __s2);
errno_t strncpy_s(char *__AXC_RESTRICT __s1, rsize_t __s1max, const char *__AXC_RESTRICT __s2, rsize_t __n);
errno_t strcat_s(char *__AXC_RESTRICT __s1, rsize_t __s1max, const char *__AXC_RESTRICT __s2);
errno_t strncat_s(char *__AXC_RESTRICT __s1, rsize_t __s1max, const char *__AXC_RESTRICT __s2, rsize_t __n);
char *strtok_s(char *__AXC_RESTRICT __s1, rsize_t *__AXC_RESTRICT __s1max, const char *__AXC_RESTRICT __s2,
               char **__AXC_RESTRICT __ptr);
errno_t memset_s(void *__s, rsize_t __smax, int __c, rsize_t __n);
errno_t strerror_s(char *__s, rsize_t __maxsize, errno_t __errnum);
size_t strerrorlen_s(errno_t __errnum);
size_t strnlen_s(const char *__s, size_t __maxsize);
#endif

#endif

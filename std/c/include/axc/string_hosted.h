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

int strcoll(const char *s1, const char *s2);
size_t strxfrm(char *__AXC_RESTRICT s1, const char *__AXC_RESTRICT s2, size_t n);
char *strerror(int errnum);
#if __STDC_VERSION__ >= 202311L
char *strdup(const char *s);
char *strndup(const char *s, size_t n);
#endif

#endif

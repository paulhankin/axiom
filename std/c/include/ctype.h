/* ctype.h - character handling (C89 4.3, C99 7.4, C11 7.4, C23 7.4), for
   the Axiom compiler's C library. The classes are those of the "C" locale
   in every locale: the library's other locales have the UTF-8 multibyte
   encoding, whose bytes past 127 are no characters of these classes. Each
   function takes EOF and every unsigned char value
   (std/c/lib/core/<name>.c, through one table, __axc_ctype_tab.c). */
#ifndef __AXIOM_CTYPE_H
#define __AXIOM_CTYPE_H

int isalnum(int __c);
int isalpha(int __c);
#if __STDC_VERSION__ >= 199901L
int isblank(int __c);
#endif
int iscntrl(int __c);
int isdigit(int __c);
int isgraph(int __c);
int islower(int __c);
int isprint(int __c);
int ispunct(int __c);
int isspace(int __c);
int isupper(int __c);
int isxdigit(int __c);
int tolower(int __c);
int toupper(int __c);

#endif

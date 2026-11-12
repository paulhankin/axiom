/* ctype.h - character handling (C89 4.3, C99 7.4, C11 7.4, C23 7.4), for
   the Axiom compiler's C library. The classes are those of the "C" locale
   in every locale: the library's other locales have the UTF-8 multibyte
   encoding, whose bytes past 127 are no characters of these classes. Each
   function takes EOF and every unsigned char value
   (std/c/lib/core/<name>.c, through one table, __axc_ctype_tab.c). */
#ifndef __AXIOM_CTYPE_H
#define __AXIOM_CTYPE_H

int isalnum(int c);
int isalpha(int c);
#if __STDC_VERSION__ >= 199901L
int isblank(int c);
#endif
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);

#endif

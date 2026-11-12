/* axc/locale.h - what the C library's other parts ask of the current
   locale (the text part, std/c/lib/text/, owns these). Private to the
   library and its headers; the names are reserved (__axc_).

   The multibyte encoding of LC_CTYPE is single-byte in the "C" and "POSIX"
   locales (each byte is one character, whose wide value is the byte's
   value) and UTF-8 in every other locale setlocale accepts. Every locale
   has the "C" locale's other conventions ('.' as the decimal point, no
   grouping, the "C" names of days and months). For <stdlib.h>:

       #define MB_CUR_MAX ((size_t)__axc_mb_cur_max())
*/
#ifndef __AXIOM_AXC_LOCALE_H
#define __AXIOM_AXC_LOCALE_H

/* 1 if LC_CTYPE's multibyte encoding is UTF-8, 0 if it is single-byte */
int __axc_mb_utf8(void);

/* the most bytes of a multibyte character in LC_CTYPE's encoding: 1 or 4 */
int __axc_mb_cur_max(void);

#endif

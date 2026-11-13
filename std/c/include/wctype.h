/* wctype.h - wide character classification and mapping utilities (C95,
   C99 7.25, C11 7.30, C23 7.32), for the Axiom compiler's C library. In a
   single-byte locale ("C", "POSIX") only ASCII characters have classes and
   case mappings, as <ctype.h>'s in the "C" locale; in a UTF-8 locale every
   code point has those of Unicode's properties (docs/c-text.md, "Wide
   character classes"). */
#ifndef __AXIOM_WCTYPE_H
#define __AXIOM_WCTYPE_H

#if __STDC_VERSION__ < 199409L
#error "<wctype.h> is a header of C95 and later"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_WCTYPE_H__ 202311L
#endif

#define __AXIOM_WANT_WINT_T
#include <axc/types.h>

/* (a class's bits, axc/text.h's __AXC_W_*; a mapping's number) */
typedef unsigned long wctype_t;
typedef int wctrans_t;

#define WEOF ((wint_t)-1)

int iswalnum(wint_t __wc);
int iswalpha(wint_t __wc);
#if __STDC_VERSION__ >= 199901L
int iswblank(wint_t __wc);
#endif
int iswcntrl(wint_t __wc);
int iswdigit(wint_t __wc);
int iswgraph(wint_t __wc);
int iswlower(wint_t __wc);
int iswprint(wint_t __wc);
int iswpunct(wint_t __wc);
int iswspace(wint_t __wc);
int iswupper(wint_t __wc);
int iswxdigit(wint_t __wc);
int iswctype(wint_t __wc, wctype_t __desc);
wctype_t wctype(const char *__property);

wint_t towlower(wint_t __wc);
wint_t towupper(wint_t __wc);
wint_t towctrans(wint_t __wc, wctrans_t __desc);
wctrans_t wctrans(const char *__property);

#endif

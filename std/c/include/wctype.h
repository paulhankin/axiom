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

int iswalnum(wint_t wc);
int iswalpha(wint_t wc);
#if __STDC_VERSION__ >= 199901L
int iswblank(wint_t wc);
#endif
int iswcntrl(wint_t wc);
int iswdigit(wint_t wc);
int iswgraph(wint_t wc);
int iswlower(wint_t wc);
int iswprint(wint_t wc);
int iswpunct(wint_t wc);
int iswspace(wint_t wc);
int iswupper(wint_t wc);
int iswxdigit(wint_t wc);
int iswctype(wint_t wc, wctype_t desc);
wctype_t wctype(const char *property);

wint_t towlower(wint_t wc);
wint_t towupper(wint_t wc);
wint_t towctrans(wint_t wc, wctrans_t desc);
wctrans_t wctrans(const char *property);

#endif

/* locale.h - localization (C89 4.4, C99 7.11, C11 7.11, C23 7.11), for the
   Axiom compiler's C library. Every locale has the "C" locale's conventions;
   they differ in the multibyte encoding of LC_CTYPE, which is single-byte
   in "C" and "POSIX" and UTF-8 in "C.UTF-8", in a name ending in ".UTF-8"
   or ".utf8", and in what "" names (docs/c-text.md, "Locales"). */
#ifndef __AXIOM_LOCALE_H
#define __AXIOM_LOCALE_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_LOCALE_H__ 202311L
#endif

#define NULL ((void *)0)

/* the categories (setlocale's first argument) */
#define LC_ALL 0
#define LC_COLLATE 1
#define LC_CTYPE 2
#define LC_MONETARY 3
#define LC_NUMERIC 4
#define LC_TIME 5

/* the formatting conventions (7.11.2.1); the members C99 added have
   reserved names before it */
#if __STDC_VERSION__ >= 199901L
#define __AXIOM_LCONV_C99(name) name
#else
#define __AXIOM_LCONV_C99(name) __axiom_##name
#endif
struct lconv {
    char *decimal_point;
    char *thousands_sep;
    char *grouping;
    char *mon_decimal_point;
    char *mon_thousands_sep;
    char *mon_grouping;
    char *positive_sign;
    char *negative_sign;
    char *currency_symbol;
    char frac_digits;
    char p_cs_precedes;
    char n_cs_precedes;
    char p_sep_by_space;
    char n_sep_by_space;
    char p_sign_posn;
    char n_sign_posn;
    char *int_curr_symbol;
    char int_frac_digits;
    char __AXIOM_LCONV_C99(int_p_cs_precedes);
    char __AXIOM_LCONV_C99(int_n_cs_precedes);
    char __AXIOM_LCONV_C99(int_p_sep_by_space);
    char __AXIOM_LCONV_C99(int_n_sep_by_space);
    char __AXIOM_LCONV_C99(int_p_sign_posn);
    char __AXIOM_LCONV_C99(int_n_sign_posn);
};
#undef __AXIOM_LCONV_C99

char *setlocale(int category, const char *locale);
struct lconv *localeconv(void);

#endif

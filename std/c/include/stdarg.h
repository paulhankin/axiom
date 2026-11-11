/* stdarg.h - variable arguments (C89 7.8, C99 7.15, C11 7.16, C23 7.16),
   for the Axiom compiler's C front end. */
#ifndef __AXIOM_STDARG_H
#define __AXIOM_STDARG_H

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_STDARG_H__ 202311L
#endif

typedef __builtin_va_list va_list;

#if __STDC_VERSION__ >= 202311L
/* (C23: the arguments after the first are not evaluated) */
#define va_start(ap, ...) __builtin_va_start(ap, 0)
#else
#define va_start(ap, parmN) __builtin_va_start(ap, parmN)
#endif
#define va_arg(ap, type) __builtin_va_arg(ap, type)
#define va_end(ap) __builtin_va_end(ap)
#if __STDC_VERSION__ >= 199901L
#define va_copy(dest, src) __builtin_va_copy(dest, src)
#endif

#endif

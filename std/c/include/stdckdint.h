/* stdckdint.h - checked integer arithmetic (C23 7.20), for the Axiom
   compiler's C front end: the macros are the compiler's builtins, which
   take any integer types but plain char, bool and enumerations, compute
   the mathematical result, store it wrapped into *result, and are true
   when it does not fit. */
#ifndef __AXIOM_STDCKDINT_H
#define __AXIOM_STDCKDINT_H

#if __STDC_VERSION__ < 202311L
#error "<stdckdint.h> is a header of C23"
#endif

#define __STDC_VERSION_STDCKDINT_H__ 202311L

#define ckd_add(result, a, b) __builtin_ckd_add(result, a, b)
#define ckd_sub(result, a, b) __builtin_ckd_sub(result, a, b)
#define ckd_mul(result, a, b) __builtin_ckd_mul(result, a, b)

#endif

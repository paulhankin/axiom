/* fenv.h - the floating-point environment (C99 7.6, C11 7.6, C17 7.6, C23
   7.6), for the Axiom compiler's C library (std/c/lib/math/fenv.c;
   docs/c.md, "<fenv.h>").

   x86-64: the SSE unit's MXCSR (all four rounding modes, the five
   exceptions); AArch64: FPCR and FPSR (likewise). WebAssembly has no
   floating-point environment: the rounding mode is to nearest and cannot be
   changed (fesetround fails for the other modes), no exception flag is ever
   set (fetestexcept is 0, feraiseexcept fails for a nonzero argument).

   The optimizer assumes the default environment: it folds floating-point
   constant expressions as if rounding to nearest with no flags raised, and
   #pragma STDC FENV_ACCESS is accepted and has no effect; see docs/c.md. */
#ifndef __AXIOM_FENV_H
#define __AXIOM_FENV_H

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L
#error "<fenv.h> is a header of C99"
#endif

#if __STDC_VERSION__ >= 202311L
#define __STDC_VERSION_FENV_H__ 202311L
#endif

/* the control (rounding mode, exception masks) and status (flags)
   registers' values */
typedef struct {
    unsigned long __axiom_control;
    unsigned long __axiom_status;
    unsigned long __axiom_dec; /* the decimal rounding direction (FE_DEC_TONEAREST ...) */
} fenv_t;

typedef unsigned short fexcept_t;

#define FE_INVALID 0x01
#define FE_DIVBYZERO 0x04
#define FE_OVERFLOW 0x08
#define FE_UNDERFLOW 0x10
#define FE_INEXACT 0x20
#define FE_ALL_EXCEPT 0x3d

#define FE_TONEAREST 0
#define FE_DOWNWARD 0x400
#define FE_UPWARD 0x800
#define FE_TOWARDZERO 0xc00

extern const fenv_t __axc_fe_dfl_env;
#define FE_DFL_ENV (&__axc_fe_dfl_env)

int feclearexcept(int __e);
int fegetexceptflag(fexcept_t *__f, int __e);
int feraiseexcept(int __e);
int fesetexceptflag(const fexcept_t *__f, int __e);
int fetestexcept(int __e);
int fegetround(void);
int fesetround(int __r);
int fegetenv(fenv_t *__env);
int feholdexcept(fenv_t *__env);
int fesetenv(const fenv_t *__env);
int feupdateenv(const fenv_t *__env);

#if __STDC_VERSION__ >= 202311L
/* the dynamic control modes: the rounding mode (and the masks), and the
   decimal rounding direction */
typedef struct {
    unsigned long __axiom_control;
    unsigned long __axiom_dec;
} femode_t;
extern const femode_t __axc_fe_dfl_mode;
#define FE_DFL_MODE (&__axc_fe_dfl_mode)
/* (C23 7.6: the operand of #pragma STDC FENV_ROUND that names the dynamic
   mode, which fegetround gives; not a rounding direction fesetround takes) */
#define FE_DYNAMIC 0x1000
int fegetmode(femode_t *__m);
int fesetmode(const femode_t *__m);
int fesetexcept(int __e);
int fetestexceptflag(const fexcept_t *__f, int __e);

/* the decimal floating types' rounding direction (7.6.7), kept apart from
   the binary one: the decimal arithmetic is the library's (in software, on
   every target, WebAssembly too); its exceptions are raised in the flags
   above where the target has them */
#define FE_DEC_TONEAREST 0
#define FE_DEC_DOWNWARD 1
#define FE_DEC_UPWARD 2
#define FE_DEC_TOWARDZERO 3
#define FE_DEC_TONEARESTFROMZERO 4
int fe_dec_getround(void);
int fe_dec_setround(int __r);
#endif

#endif

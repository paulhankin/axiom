/* fenv.c - <fenv.h> (C17 7.6, C23 7.6), for x86-64's MXCSR and AArch64's
   FPCR and FPSR (the registers' layouts: fe_raw.c). WebAssembly has no
   floating-point environment: rounding to nearest, no flags; the functions
   succeed where that is what they are asked for and fail otherwise. */
#if __STDC_VERSION__ >= 199901L
#include <fenv.h>

#define KIND (__builtin_axiom_fenv_kind())

unsigned long __axc_fe_flags(void);
int __axc_fe_set_flags(unsigned long f, unsigned long mask);
int __axc_fe_round(void);
/* the decimal rounding direction (std/c/rt/dfp_env.ax) */
unsigned __axiom_int64 __axc_dec_round_get(void);
unsigned __axiom_int64 __axc_dec_round_set(unsigned __axiom_int64 m);

/* the environment of program start-up: rounding to nearest (binary and
   decimal), no flags, all exceptions masked (MXCSR 0x1f80; FPCR 0) */
const fenv_t __axc_fe_dfl_env = { 0, 0, 0 };

static int valid(int e)
{
    return (e & ~FE_ALL_EXCEPT) == 0;
}

int feclearexcept(int e)
{
    if (!valid(e))
        return 1;
    return __axc_fe_set_flags(0, (unsigned long)e);
}

int fegetexceptflag(fexcept_t *f, int e)
{
    if (!valid(e))
        return 1;
    *f = (fexcept_t)(__axc_fe_flags() & (unsigned long)e);
    return 0;
}

int feraiseexcept(int e)
{
    if (!valid(e))
        return 1;
    /* (raising overflow or underflow does not also raise inexact) */
    return __axc_fe_set_flags((unsigned long)e, (unsigned long)e);
}

int fesetexceptflag(const fexcept_t *f, int e)
{
    if (!valid(e))
        return 1;
    return __axc_fe_set_flags((unsigned long)*f, (unsigned long)e);
}

int fetestexcept(int e)
{
    return (int)(__axc_fe_flags() & (unsigned long)e & FE_ALL_EXCEPT);
}

int fegetround(void)
{
    return __axc_fe_round();
}

int fesetround(int r)
{
    unsigned __axiom_int64 v;
    if (r != FE_TONEAREST && r != FE_DOWNWARD && r != FE_UPWARD && r != FE_TOWARDZERO)
        return 1;
    if (KIND == 1) {
        v = __builtin_axiom_fenv_get();
        v = (v & ~(unsigned __axiom_int64)0x6000) | ((unsigned __axiom_int64)r << 3);
        __builtin_axiom_fenv_set(v);
        return 0;
    }
    if (KIND == 2) {
        unsigned long m = (unsigned long)r >> 10;
        if (m == 1 || m == 2)
            m ^= 3; /* down is RM (2), up RP (1) */
        v = __builtin_axiom_fenv_get();
        v = (v & ~(unsigned __axiom_int64)0xc00000) | ((unsigned __axiom_int64)m << 22);
        __builtin_axiom_fenv_set(v);
        return 0;
    }
    return r != FE_TONEAREST;
}

/* fenv_t: the control part (MXCSR's masks and rounding control, or FPCR)
   and the flags (in the library's FE_ numbering) */
static void get_env(fenv_t *env)
{
    unsigned __axiom_int64 v = __builtin_axiom_fenv_get();
    if (KIND == 1)
        env->__axiom_control = (unsigned long)(v & 0xffc0) ^ 0x1f80;
    else
        env->__axiom_control = (unsigned long)(v & 0xffffffff);
    env->__axiom_status = __axc_fe_flags();
    env->__axiom_dec = (unsigned long)__axc_dec_round_get();
}

/* (the control part is stored relative to the default, so that
   FE_DFL_ENV's zeros are the default on each target) */
static int set_env(const fenv_t *env)
{
    unsigned __axiom_int64 v;
    if (__axc_dec_round_set(env->__axiom_dec) != 0)
        return 1;
    if (KIND == 1) {
        v = __builtin_axiom_fenv_get();
        v = (v & ~(unsigned __axiom_int64)0xffc0) | ((env->__axiom_control ^ 0x1f80) & 0xffc0);
        __builtin_axiom_fenv_set(v);
    } else if (KIND == 2) {
        v = __builtin_axiom_fenv_get();
        v = (v & ~(unsigned __axiom_int64)0xffffffff) | (env->__axiom_control & 0xffffffff);
        __builtin_axiom_fenv_set(v);
    }
    return __axc_fe_set_flags(env->__axiom_status, FE_ALL_EXCEPT);
}

int fegetenv(fenv_t *env)
{
    get_env(env);
    return 0;
}

int feholdexcept(fenv_t *env)
{
    /* the flags cleared; exceptions are never trapped (non-stop mode is
       the only one) */
    get_env(env);
    return __axc_fe_set_flags(0, FE_ALL_EXCEPT);
}

int fesetenv(const fenv_t *env)
{
    return set_env(env);
}

int feupdateenv(const fenv_t *env)
{
    unsigned long f = __axc_fe_flags();
    int r = set_env(env);
    return r | __axc_fe_set_flags(f, f);
}

/* C23's (the library's units are C17: the type is <fenv.h>'s C23 one) */
#ifndef __STDC_VERSION_FENV_H__
typedef struct {
    unsigned long __axiom_control;
    unsigned long __axiom_dec;
} femode_t;
#endif
const femode_t __axc_fe_dfl_mode = { 0, 0 };

int fegetmode(femode_t *m)
{
    fenv_t e;
    get_env(&e);
    m->__axiom_control = e.__axiom_control;
    m->__axiom_dec = e.__axiom_dec;
    return 0;
}

int fesetmode(const femode_t *m)
{
    fenv_t e;
    get_env(&e);
    e.__axiom_control = m->__axiom_control;
    e.__axiom_dec = m->__axiom_dec;
    return set_env(&e);
}

/* C23 7.6.7: the decimal rounding direction, on every target */
int fe_dec_getround(void)
{
    return (int)__axc_dec_round_get();
}

int fe_dec_setround(int r)
{
    if (r < 0)
        return 1;
    return (int)__axc_dec_round_set((unsigned __axiom_int64)r);
}

int fesetexcept(int e)
{
    if (!valid(e))
        return 1;
    return __axc_fe_set_flags((unsigned long)e, (unsigned long)e);
}

int fetestexceptflag(const fexcept_t *f, int e)
{
    return (int)(*f & (unsigned long)e & FE_ALL_EXCEPT);
}

#else
typedef int __axc_fenv_c89; /* (no <fenv.h> before C99) */
#endif

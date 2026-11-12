/* fe_raw.c - the floating-point environment's registers as the library
   numbers them (fenv.h's FE_ values), for <fenv.h> (fenv.c) and for the
   errors of <math.h>'s functions (m_err.c). Written without <fenv.h>, so
   that it builds in every standard (the functions of <math.h> exist in
   C89 too).

   __builtin_axiom_fenv_get() is MXCSR on x86-64, FPCR | FPSR << 32 on
   AArch64, 0 on WebAssembly (__builtin_axiom_fenv_kind() 1, 2, 0; see
   cmd/axiom/src/c/csema_fp.ax).
   MXCSR: flags IE DE ZE OE UE PE in bits 0-5 (the library's FE_ values are
   these bits, DE left out), masks in 7-12, the rounding control RC in bits
   13-14 (0 nearest, 1 down, 2 up, 3 toward zero), so FE_DOWNWARD (0x400)
   ... are RC << 10.
   FPSR: IOC DZC OFC UFC IXC in bits 0-4; FPCR: RMode in bits 22-23 (0
   nearest, 1 up, 2 down, 3 toward zero). */

#define KIND (__builtin_axiom_fenv_kind())

/* the library's flags (FE_INVALID ...) as FPSR's bits, and back */
static unsigned long to_fpsr(unsigned long f)
{
    return (f & 1) | ((f >> 1) & 0x1e);
}

static unsigned long from_fpsr(unsigned long s)
{
    return (s & 1) | ((s & 0x1e) << 1);
}

/* the flags set */
unsigned long __axc_fe_flags(void)
{
    unsigned __axiom_int64 v = __builtin_axiom_fenv_get();
    if (KIND == 1)
        return (unsigned long)(v & 0x3d);
    if (KIND == 2)
        return from_fpsr((unsigned long)(v >> 32));
    return 0;
}

/* sets the flags to f in the set mask (0 where flags do not exist) */
int __axc_fe_set_flags(unsigned long f, unsigned long mask)
{
    unsigned __axiom_int64 v = __builtin_axiom_fenv_get();
    f &= mask & 0x3d;
    if (KIND == 1) {
        v = (v & ~(unsigned __axiom_int64)(mask & 0x3d)) | f;
        __builtin_axiom_fenv_set(v);
        return 0;
    }
    if (KIND == 2) {
        unsigned __axiom_int64 s = to_fpsr(mask & 0x3d);
        v = (v & ~(s << 32)) | ((unsigned __axiom_int64)to_fpsr(f) << 32);
        __builtin_axiom_fenv_set(v);
        return 0;
    }
    return f != 0;
}

/* raises the flags f (sets them: the exceptions are never trapped) */
void __axc_fe_raise(unsigned int f)
{
    if (KIND != 0)
        (void)__axc_fe_set_flags((unsigned long)f, (unsigned long)f);
}

/* the rounding mode, as FE_TONEAREST ... */
int __axc_fe_round(void)
{
    unsigned __axiom_int64 v = __builtin_axiom_fenv_get();
    if (KIND == 1)
        return (int)((v >> 3) & 0xc00);
    if (KIND == 2) {
        unsigned long m = (unsigned long)(v >> 22) & 3;
        /* RP (1) is upward, RM (2) downward */
        if (m == 1 || m == 2)
            m ^= 3;
        return (int)(m << 10);
    }
    return 0;
}

/* m_err.c - the errors of <math.h>'s functions (C17 7.12.1), which those
   written in Axiom (std/c/rt/math*.ax) report through this: errno
   (math_errhandling has MATH_ERRNO) and the floating-point exception flags
   (MATH_ERREXCEPT, where the target has them: fe_raw.c). */
#include <errno.h>

void __axc_fe_raise(unsigned int f);

void __axc_math_err(int e, unsigned int flags)
{
    if (e != 0)
        errno = e;
    if (flags != 0)
        __axc_fe_raise(flags);
}

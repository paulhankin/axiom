/* fn_cx_f32x.c - <complex.h>'s _Float32x functions (C23 Annex H.11): those of the
   standard (or _Float128) type of its format, as fn_f32x.c's real ones. Generated
   by cmd/internal/tools/gen_floatn.py: do not edit. */
#if __STDC_VERSION__ >= 199901L
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <complex.h>
_Float32x _Complex (cacosf32x)(_Float32x _Complex __z)
{
    return (cacos)(__z);
}

_Float32x _Complex (casinf32x)(_Float32x _Complex __z)
{
    return (casin)(__z);
}

_Float32x _Complex (catanf32x)(_Float32x _Complex __z)
{
    return (catan)(__z);
}

_Float32x _Complex (ccosf32x)(_Float32x _Complex __z)
{
    return (ccos)(__z);
}

_Float32x _Complex (csinf32x)(_Float32x _Complex __z)
{
    return (csin)(__z);
}

_Float32x _Complex (ctanf32x)(_Float32x _Complex __z)
{
    return (ctan)(__z);
}

_Float32x _Complex (cacoshf32x)(_Float32x _Complex __z)
{
    return (cacosh)(__z);
}

_Float32x _Complex (casinhf32x)(_Float32x _Complex __z)
{
    return (casinh)(__z);
}

_Float32x _Complex (catanhf32x)(_Float32x _Complex __z)
{
    return (catanh)(__z);
}

_Float32x _Complex (ccoshf32x)(_Float32x _Complex __z)
{
    return (ccosh)(__z);
}

_Float32x _Complex (csinhf32x)(_Float32x _Complex __z)
{
    return (csinh)(__z);
}

_Float32x _Complex (ctanhf32x)(_Float32x _Complex __z)
{
    return (ctanh)(__z);
}

_Float32x _Complex (cexpf32x)(_Float32x _Complex __z)
{
    return (cexp)(__z);
}

_Float32x _Complex (clogf32x)(_Float32x _Complex __z)
{
    return (clog)(__z);
}

_Float32x _Complex (csqrtf32x)(_Float32x _Complex __z)
{
    return (csqrt)(__z);
}

_Float32x _Complex (conjf32x)(_Float32x _Complex __z)
{
    return (conj)(__z);
}

_Float32x _Complex (cprojf32x)(_Float32x _Complex __z)
{
    return (cproj)(__z);
}

_Float32x (cabsf32x)(_Float32x _Complex __z)
{
    return (cabs)(__z);
}

_Float32x (cargf32x)(_Float32x _Complex __z)
{
    return (carg)(__z);
}

_Float32x (cimagf32x)(_Float32x _Complex __z)
{
    return (cimag)(__z);
}

_Float32x (crealf32x)(_Float32x _Complex __z)
{
    return (creal)(__z);
}

_Float32x _Complex (cpowf32x)(_Float32x _Complex __z, _Float32x _Complex __w)
{
    return (cpow)(__z, __w);
}

#else
typedef int __axc_fn_cx_f32x_c89;
#endif

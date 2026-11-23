/* fn_cx_f64.c - <complex.h>'s _Float64 functions (C23 Annex H.11): those of the
   standard (or _Float128) type of its format, as fn_f64.c's real ones. Generated
   by cmd/internal/tools/gen_floatn.py: do not edit. */
#if __STDC_VERSION__ >= 199901L
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <complex.h>
_Float64 _Complex (cacosf64)(_Float64 _Complex __z)
{
    return (cacos)(__z);
}

_Float64 _Complex (casinf64)(_Float64 _Complex __z)
{
    return (casin)(__z);
}

_Float64 _Complex (catanf64)(_Float64 _Complex __z)
{
    return (catan)(__z);
}

_Float64 _Complex (ccosf64)(_Float64 _Complex __z)
{
    return (ccos)(__z);
}

_Float64 _Complex (csinf64)(_Float64 _Complex __z)
{
    return (csin)(__z);
}

_Float64 _Complex (ctanf64)(_Float64 _Complex __z)
{
    return (ctan)(__z);
}

_Float64 _Complex (cacoshf64)(_Float64 _Complex __z)
{
    return (cacosh)(__z);
}

_Float64 _Complex (casinhf64)(_Float64 _Complex __z)
{
    return (casinh)(__z);
}

_Float64 _Complex (catanhf64)(_Float64 _Complex __z)
{
    return (catanh)(__z);
}

_Float64 _Complex (ccoshf64)(_Float64 _Complex __z)
{
    return (ccosh)(__z);
}

_Float64 _Complex (csinhf64)(_Float64 _Complex __z)
{
    return (csinh)(__z);
}

_Float64 _Complex (ctanhf64)(_Float64 _Complex __z)
{
    return (ctanh)(__z);
}

_Float64 _Complex (cexpf64)(_Float64 _Complex __z)
{
    return (cexp)(__z);
}

_Float64 _Complex (clogf64)(_Float64 _Complex __z)
{
    return (clog)(__z);
}

_Float64 _Complex (csqrtf64)(_Float64 _Complex __z)
{
    return (csqrt)(__z);
}

_Float64 _Complex (conjf64)(_Float64 _Complex __z)
{
    return (conj)(__z);
}

_Float64 _Complex (cprojf64)(_Float64 _Complex __z)
{
    return (cproj)(__z);
}

_Float64 (cabsf64)(_Float64 _Complex __z)
{
    return (cabs)(__z);
}

_Float64 (cargf64)(_Float64 _Complex __z)
{
    return (carg)(__z);
}

_Float64 (cimagf64)(_Float64 _Complex __z)
{
    return (cimag)(__z);
}

_Float64 (crealf64)(_Float64 _Complex __z)
{
    return (creal)(__z);
}

_Float64 _Complex (cpowf64)(_Float64 _Complex __z, _Float64 _Complex __w)
{
    return (cpow)(__z, __w);
}

#else
typedef int __axc_fn_cx_f64_c89;
#endif

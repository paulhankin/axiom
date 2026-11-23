/* fn_cx_f64x.c - <complex.h>'s _Float64x functions (C23 Annex H.11): those of the
   standard (or _Float128) type of its format, as fn_f64x.c's real ones. Generated
   by cmd/internal/tools/gen_floatn.py: do not edit. */
#if __STDC_VERSION__ >= 199901L
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include <complex.h>
#if __AXIOM_LDBL_WIDTH__ != 64
_Float64x _Complex (cacosf64x)(_Float64x _Complex __z)
{
    return (cacosl)(__z);
}

_Float64x _Complex (casinf64x)(_Float64x _Complex __z)
{
    return (casinl)(__z);
}

_Float64x _Complex (catanf64x)(_Float64x _Complex __z)
{
    return (catanl)(__z);
}

_Float64x _Complex (ccosf64x)(_Float64x _Complex __z)
{
    return (ccosl)(__z);
}

_Float64x _Complex (csinf64x)(_Float64x _Complex __z)
{
    return (csinl)(__z);
}

_Float64x _Complex (ctanf64x)(_Float64x _Complex __z)
{
    return (ctanl)(__z);
}

_Float64x _Complex (cacoshf64x)(_Float64x _Complex __z)
{
    return (cacoshl)(__z);
}

_Float64x _Complex (casinhf64x)(_Float64x _Complex __z)
{
    return (casinhl)(__z);
}

_Float64x _Complex (catanhf64x)(_Float64x _Complex __z)
{
    return (catanhl)(__z);
}

_Float64x _Complex (ccoshf64x)(_Float64x _Complex __z)
{
    return (ccoshl)(__z);
}

_Float64x _Complex (csinhf64x)(_Float64x _Complex __z)
{
    return (csinhl)(__z);
}

_Float64x _Complex (ctanhf64x)(_Float64x _Complex __z)
{
    return (ctanhl)(__z);
}

_Float64x _Complex (cexpf64x)(_Float64x _Complex __z)
{
    return (cexpl)(__z);
}

_Float64x _Complex (clogf64x)(_Float64x _Complex __z)
{
    return (clogl)(__z);
}

_Float64x _Complex (csqrtf64x)(_Float64x _Complex __z)
{
    return (csqrtl)(__z);
}

_Float64x _Complex (conjf64x)(_Float64x _Complex __z)
{
    return (conjl)(__z);
}

_Float64x _Complex (cprojf64x)(_Float64x _Complex __z)
{
    return (cprojl)(__z);
}

_Float64x (cabsf64x)(_Float64x _Complex __z)
{
    return (cabsl)(__z);
}

_Float64x (cargf64x)(_Float64x _Complex __z)
{
    return (cargl)(__z);
}

_Float64x (cimagf64x)(_Float64x _Complex __z)
{
    return (cimagl)(__z);
}

_Float64x (crealf64x)(_Float64x _Complex __z)
{
    return (creall)(__z);
}

_Float64x _Complex (cpowf64x)(_Float64x _Complex __z, _Float64x _Complex __w)
{
    return (cpowl)(__z, __w);
}

#else
_Float64x _Complex (cacosf64x)(_Float64x _Complex __z)
{
    return (cacosf128)(__z);
}

_Float64x _Complex (casinf64x)(_Float64x _Complex __z)
{
    return (casinf128)(__z);
}

_Float64x _Complex (catanf64x)(_Float64x _Complex __z)
{
    return (catanf128)(__z);
}

_Float64x _Complex (ccosf64x)(_Float64x _Complex __z)
{
    return (ccosf128)(__z);
}

_Float64x _Complex (csinf64x)(_Float64x _Complex __z)
{
    return (csinf128)(__z);
}

_Float64x _Complex (ctanf64x)(_Float64x _Complex __z)
{
    return (ctanf128)(__z);
}

_Float64x _Complex (cacoshf64x)(_Float64x _Complex __z)
{
    return (cacoshf128)(__z);
}

_Float64x _Complex (casinhf64x)(_Float64x _Complex __z)
{
    return (casinhf128)(__z);
}

_Float64x _Complex (catanhf64x)(_Float64x _Complex __z)
{
    return (catanhf128)(__z);
}

_Float64x _Complex (ccoshf64x)(_Float64x _Complex __z)
{
    return (ccoshf128)(__z);
}

_Float64x _Complex (csinhf64x)(_Float64x _Complex __z)
{
    return (csinhf128)(__z);
}

_Float64x _Complex (ctanhf64x)(_Float64x _Complex __z)
{
    return (ctanhf128)(__z);
}

_Float64x _Complex (cexpf64x)(_Float64x _Complex __z)
{
    return (cexpf128)(__z);
}

_Float64x _Complex (clogf64x)(_Float64x _Complex __z)
{
    return (clogf128)(__z);
}

_Float64x _Complex (csqrtf64x)(_Float64x _Complex __z)
{
    return (csqrtf128)(__z);
}

_Float64x _Complex (conjf64x)(_Float64x _Complex __z)
{
    return (conjf128)(__z);
}

_Float64x _Complex (cprojf64x)(_Float64x _Complex __z)
{
    return (cprojf128)(__z);
}

_Float64x (cabsf64x)(_Float64x _Complex __z)
{
    return (cabsf128)(__z);
}

_Float64x (cargf64x)(_Float64x _Complex __z)
{
    return (cargf128)(__z);
}

_Float64x (cimagf64x)(_Float64x _Complex __z)
{
    return (cimagf128)(__z);
}

_Float64x (crealf64x)(_Float64x _Complex __z)
{
    return (crealf128)(__z);
}

_Float64x _Complex (cpowf64x)(_Float64x _Complex __z, _Float64x _Complex __w)
{
    return (cpowf128)(__z, __w);
}

#endif
#else
typedef int __axc_fn_cx_f64x_c89;
#endif

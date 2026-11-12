/* cfloat.c - the float forms of <complex.h>'s functions: the double form's
   result (computed from the exactly converted argument), each part rounded
   to float. */
#if __STDC_VERSION__ >= 199901L
#include <complex.h>

float _Complex cacosf(float _Complex z)
{
    double _Complex w = cacos(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex casinf(float _Complex z)
{
    double _Complex w = casin(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex catanf(float _Complex z)
{
    double _Complex w = catan(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex ccosf(float _Complex z)
{
    double _Complex w = ccos(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex csinf(float _Complex z)
{
    double _Complex w = csin(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex ctanf(float _Complex z)
{
    double _Complex w = ctan(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex cacoshf(float _Complex z)
{
    double _Complex w = cacosh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex casinhf(float _Complex z)
{
    double _Complex w = casinh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex catanhf(float _Complex z)
{
    double _Complex w = catanh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex ccoshf(float _Complex z)
{
    double _Complex w = ccosh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex csinhf(float _Complex z)
{
    double _Complex w = csinh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex ctanhf(float _Complex z)
{
    double _Complex w = ctanh(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex cexpf(float _Complex z)
{
    double _Complex w = cexp(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex clogf(float _Complex z)
{
    double _Complex w = clog(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex csqrtf(float _Complex z)
{
    double _Complex w = csqrt(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex (conjf)(float _Complex z)
{
    double _Complex w = conj(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float _Complex cprojf(float _Complex z)
{
    double _Complex w = cproj(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
    return __builtin_complex((float)__builtin_creal(w), (float)__builtin_cimag(w));
}

float cabsf(float _Complex z)
{
    return (float)cabs(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
}

float cargf(float _Complex z)
{
    return (float)carg(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
}

float (cimagf)(float _Complex z)
{
    return (float)cimag(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
}

float (crealf)(float _Complex z)
{
    return (float)creal(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)));
}

float _Complex cpowf(float _Complex z, float _Complex w)
{
    double _Complex r = cpow(__builtin_complex((double)__builtin_creal(z), (double)__builtin_cimag(z)),
                             __builtin_complex((double)__builtin_creal(w), (double)__builtin_cimag(w)));
    return __builtin_complex((float)__builtin_creal(r), (float)__builtin_cimag(r));
}
#else
typedef int __axc_cfloat_c89;
#endif

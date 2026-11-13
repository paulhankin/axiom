/* The complex functions of the interchange and extended types (C23 Annex
   H.11): _Float128's (complexf128.c, in binary128's precision) and
   _Float16's (complexf16.c, the double functions' parts rounded once),
   those of _Float32 ... _Float64x (the standard functions of their
   formats), and <tgmath.h>'s choice of them for complex arguments of
   those types. */
/* flags: --c-std=c23 */
#define __STDC_WANT_IEC_60559_TYPES_EXT__ 1
#include "print.h"
#include <tgmath.h>

static int near128(_Float128 x, double want)
{
    double d = (double)x - want;
    return d < 1e-15 && d > -1e-15;
}

int main(void)
{
    _Float128 _Complex z = __builtin_complex((_Float128)0.5, (_Float128)-1.25);
    _Float16 _Complex h = __builtin_complex((_Float16)0.5, (_Float16)-1.25);
    /* values against arbitrary-precision references */
    _Float128 _Complex e = cexpf128(z);
    print_bool(near128(crealf128(e), 0.51987868600849365733) && near128(cimagf128(e), -1.5646111274988195279));
    print_bool(near128(crealf128(casinhf128(z)), 0.89889362936948251439) &&
               near128(cimagf128(ctanf128(z)), -0.90672480431943097598));
    print_bool(near128(crealf128(csqrtf128(z)), 0.96080466323379852958) && near128(cabsf128(z), 1.3462912017836260078));
    print_bool(near128(cimagf128(cpowf128(z, z)), -0.21569904878504624627));
    /* (binary128's precision: beyond double's) */
    _Float128 one = 1;
    print_bool(cabsf128(__builtin_complex(one, (_Float128)0x1p-50)) > one);
    _Float16 _Complex eh = cexpf16(h);
    print_bool(crealf16(eh) == (_Float16)0.51987868600849365733 && cimagf16(eh) == (_Float16)-1.5646111274988195279);
    print_bool(cargf16(h) == (_Float16)-1.1902899496825317 && cabsf16(h) == (_Float16)1.3462912017836260078);
    /* _Float32 ... */
    _Float32 _Complex f = cexpf32(__builtin_complex((_Float32)0.5, (_Float32)-1.25));
    print_bool(crealf32(f) == crealf(cexpf(0.5f - 1.25f * I)));
    /* <tgmath.h> */
    print_bool(_Generic(exp(z), _Float128 _Complex: 1, default: 0) && _Generic(exp(h), _Float16 _Complex: 1, default: 0));
    print_bool(_Generic(fabs(z), _Float128: 1, default: 0) && _Generic(carg(h), _Float16: 1, default: 0));
    print_bool(_Generic(pow(z, one), _Float128 _Complex: 1, default: 0) && creal(exp(z)) == crealf128(e));
    return 0;
}
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true
// expect: true

## C library: accurate complex functions

- `<complex.h>`'s functions (every type: `float`, `double`, `long double` in each of its
  formats, `_Float16`, `_Float128`) compute each part in pairs of the real type from real
  functions that give about twice its precision, and round once: each part of `cexp`,
  `clog`, `csqrt`, `csin` ... `ctanh`, `casin` ... `catanh` and `cpow` is now within
  half an ulp and a small fraction over the whole plane (were up to 2.9 ulps; measured
  against an arbitrary-precision reference near the branch cuts, the unit circle, the
  poles of `ctan` and the overflow and underflow thresholds too), with Annex G's special
  values as before. A double function takes 1.2 to 2.5 times its former time.
- Fixed: `clog` (and `casin`, `cacos`, `casinh`, `cacosh`, whose large arguments take its
  real part, `cpow` its result) of an argument beyond about 1e154 in magnitude returned an
  infinite or NaN real part; `cacos` and `cacosh` of a large argument near the real axis
  lost the low-order digits of their small part.

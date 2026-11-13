## C

- `<math.h>` has the rest of C23's functions, in the three types: `exp2m1`, `exp10m1`,
  `log2p1`, `log10p1`, `logp1`, `rootn`, `pown`, `powr`, `compoundn`, `rsqrt`, `tanpi`,
  `asinpi`, `acospi`, `atanpi`, `atan2pi`, `fromfp`, `ufromfp`, `fromfpx`, `ufromfpx`, the
  narrowing operations (`fadd` ... `ffma`, `fsqrt`, `faddl` ..., `daddl` ...: correctly
  rounded once in the current rounding direction), `getpayload`, `setpayload`,
  `setpayloadsig`, `totalorder`, `totalordermag`, `canonicalize`, `llogb`, the macros
  `iseqsig`, `FP_INT_*`, `FP_LLOGB0` and `FP_LLOGBNAN`, and `<tgmath.h>`'s macros for them.
  Where `long double` is wider than `double`, its forms of these (and `sinpil`, `cospil`)
  compute in the format's precision.
- `#pragma STDC FENV_ACCESS ON` (and the option `--c-fenv-access`) is honoured: in a
  function that has it, the optimizer does not fold, merge, move or remove floating
  operations, and constant initializers of automatic objects are evaluated at run time.
  `#pragma STDC FENV_ROUND` sets the static rounding direction of its scope (C23). The
  relational operators `<`, `<=`, `>` and `>=` now raise "invalid" for a NaN operand, as
  Annex F asks (`isless` and the like do not).
- The float functions have kernels of their own (`log2f`, `tanf`, `atanf`, `asinf`,
  `cbrtf`, `hypotf`, the hyperbolic ones, `erff`, ...): correctly rounded but in rare
  cases, and most as fast as the system library's or faster.

## Standard library

- `std/math`'s functions are now the C library's (one implementation,
  `std/internal/math/`): more accurate (within 0.5-0.77 ulp; `tan`, `expm1`, `sinh`,
  `tanh`, `erf` and `lgamma` were 2 to 25000 ulps off), and the float ones correctly
  rounded but in rare cases.

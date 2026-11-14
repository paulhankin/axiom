## C library

- The C library has `<math.h>`, `<fenv.h>` and `<tgmath.h>`, and the functions of
  `<complex.h>` (docs/c.md, "`<math.h>`, `<fenv.h>`, `<tgmath.h>` and `<complex.h>`'s
  functions"): every function of C17 7.12 and of C23 in its three types (`exp10`,
  `exp2m1`, `exp10m1`, `log2p1`, `log10p1`, `logp1`, `rootn`, `pown`, `powr`,
  `compoundn`, `rsqrt`, `sinpi`, `cospi`, `tanpi`, `asinpi`, `acospi`, `atanpi`,
  `atan2pi`, `roundeven`, `nextup`, `nextdown`, `llogb`, `fromfp` and its kin, the
  `fmaximum` family, the narrowing operations `fadd` ... `daddl` ..., `getpayload`,
  `setpayload`, `setpayloadsig`, `totalorder`, `totalordermag`, `canonicalize`, and the
  macros `iseqsig`, `FP_INT_*`, `FP_LLOGB0` and `FP_LLOGBNAN`), with `<tgmath.h>`'s
  macros for them. The elementary functions are within 0.50 to 0.77 ulp of the exact
  results over their whole domains (measured against arbitrary precision), the float ones
  within 0.5 + 2^-12 ulp (0.500 measured, with kernels of their own); the exact ones
  (`fmod`, `remquo`, the rounding functions, `fma`, `scalbn`, ...) are exact in every
  rounding mode; the results are the same bits on every target. Errors set `errno` and
  raise the floating-point exceptions (`math_errhandling`).
- A `long double` wider than `double` (the x87 format on x86-64 Linux and macOS, binary128 on AArch64
  Linux and WebAssembly) has its elementary functions in its own precision (triple-double
  arithmetic, with a faster double-double path for the x87 format that gives the same
  bits): within 0.5 + 2^-6 ulp, `erfl`, `erfcl`, `lgammal` and `tgammal` (and
  `erff128` ... `tgammaf128`) within 0.55 ulp of binary128; `fmal` and `sqrtl` correctly
  rounded, and the complex `l` functions on them. The `_Float16` elementary functions are
  correctly rounded.
- `<fenv.h>`: the rounding modes and exception flags of x86-64 (MXCSR) and AArch64 (FPCR,
  FPSR); WebAssembly has no floating-point environment (to nearest, no flags).
- `<tgmath.h>` works in C99 as in later standards (the builtin `__builtin_axiom_tg`); the
  classification and comparison macros are builtins that evaluate their operands once and
  raise no exception for a quiet NaN, `long double` of every format included.
- `NAN` is a positive quiet NaN (it had the sign of the compiling machine's default NaN).
- A program linked by the system's linker (`--link=`, `-l`) no longer includes all of the
  C library: only what its own units reach.
- Fixed in the `long double` functions: `asinhl` and `acoshl` of arguments beyond about
  2^512 were wrong; `powl(x, n)` for an integer n whose exact result fits 128 bits is
  exact and correctly rounded (ties to even); `logl(1)` (and `log2l`, `log10l`) raised a
  spurious "inexact", and `lgammal`, `log1pl`, `acosl` of some huge or tiny arguments a
  spurious "underflow"; `acosh`, `asinh`, `lgamma` and `pow` of arguments above 2^972
  raised a spurious "overflow".

## Standard library

- `std/math`'s functions are now the C library's (one implementation,
  `std/internal/math/`): more accurate (within 0.50 to 0.77 ulp; `tan`, `expm1`, `sinh`,
  `tanh`, `erf` and `lgamma` were 2 to 25000 ulps off), and the float ones correctly
  rounded but in rare cases.

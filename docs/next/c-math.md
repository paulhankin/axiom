## C

- The C library has `<math.h>`, `<fenv.h>` and `<tgmath.h>`, and the functions of
  `<complex.h>` (docs/c.md, "`<math.h>`, `<fenv.h>`, `<tgmath.h>` and `<complex.h>`'s
  functions"): every function of C17 7.12 in its three types, and C23's `exp10`,
  `roundeven`, `nextup`, `nextdown`, `sinpi`, `cospi` and the `fmaximum` family. The
  elementary functions are within 0.51-0.73 ulp of the exact results over their whole
  domains (measured against arbitrary precision), the float ones within 0.501; the exact
  ones (`fmod`, `remquo`, the rounding functions, `fma`, `scalbn`, ...) are exact in every
  rounding mode; the results are the same bits on every target. Errors set `errno` and
  raise the floating-point exceptions (`math_errhandling`). Against the x86-64 Linux
  system library most of the functions take at most about twice as long (`exp` is faster;
  `log`, `log2`, `asin`, `acos` and `sinf` up to 2.6 times).
- A `long double` wider than `double` (x86-64 Linux's x87 format, binary128 on AArch64
  Linux and WebAssembly) has its elementary functions in its own precision, within 0.5 +
  2^-6 ulp (triple-double arithmetic), `fmal` and `sqrtl` correctly rounded, and the
  complex `l` functions on them.
- `<fenv.h>`: the rounding modes and exception flags of x86-64 (MXCSR) and AArch64 (FPCR,
  FPSR); WebAssembly has no floating-point environment (to nearest, no flags). The
  optimizer still assumes the default environment (`FENV_ACCESS` is not honoured).
- `<tgmath.h>` works in C99 as in later standards (the builtin `__builtin_axiom_tg`); the
  classification and comparison macros are builtins that evaluate their operands once and
  raise no exception for a quiet NaN, `long double` of every format included.
- `NAN` is a positive quiet NaN (it had the sign of the compiling machine's default NaN).
- A program linked by the system's linker (`--link=`, `-l`) no longer includes all of the
  C library: only what its own units reach.

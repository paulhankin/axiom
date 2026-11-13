## C library and standard library: faster floating point

- The C library's `log2` takes about 1.5 times the system library's time (was 1.8),
  `acosh` 1.4 (1.65), `pow` 1.45 (1.6), `powf` 1.5 (1.6), `lgammaf` 1.25 (1.55),
  `lgamma` 1.25 (1.45); `std/math`'s `log2`, `pow` and `lgamma` are 12 to 15% faster.
  The results are the same bits as before but for `log2`'s, which differ in about one
  case in 10^4 (both within 0.5 + 2^-6 ulp; measured 0.500 ulp at worst); `acosh`,
  `asinh`, `lgamma` and `pow` of arguments above 2^972 no longer raise a spurious
  "overflow".
- x86-64 code generation: a floating-point constant used once is the memory operand of
  a scalar `addsd`/`mulsd` (and the like) without AVX too, and of a commutative VEX
  instruction whichever its place; `@fma(a, b, -c)` is a `vfmsub` and `@fma(-a, b, c)`
  a `vfnmadd`; and a function whose returns meet in one block (an inlined callee's)
  returns from each path, so that the paths that make no call need no stack frame.

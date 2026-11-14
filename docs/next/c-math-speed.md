## C library and standard library

- Against the x86-64 Linux system C library (`-O2`, the time of a call): most of the C
  library's double and float functions take 0.5 to 1.3 times its time; `erf`, `erfc`,
  `exp10`, `lgamma`, `lgammaf`, `asinh` and `tan` about 1.15 to 1.3, `acosh`, `atanh`,
  `pow` and `powf` 1.4 to 1.5, and `log2` 1.5 to 1.55 (docs/c.md, Speed). `log2`'s
  results changed in about one case in 10^4 (both within 0.5 + 2^-6 ulp); the others are
  the same bits as before their speed-ups.
- `std/math` has the same kernels: its `atan`, `atan2`, `tan`, `sinh`, `cbrt` and
  `lgamma` are about as fast as the less accurate functions they replaced (within 10%),
  with the same or better accuracy.
- The x87 format's `long double` functions (x86-64 Linux) take a double-double path with
  a rounding test before the triple-double one, the same bits: `sinl`, `cosl`, `tanl`,
  `acosl`, `powl` and `tgammal` take 0.45 to 0.8 times the x87 hardware's functions of the
  system library, the others 1.15 to 2.2 times (`logl` 1.25, `log1pl` 1.7, `lgammal`
  1.25 to 1.35, `expl` 1.8, `asinl` 1.9, `atanl` 2.0, `asinhl` 2.2). binary128's take 0.2
  to 1.2 times the system library's `_Float128` functions; its `erf` and `erfc` are 5 to
  6 times faster than before.
- `long double` arithmetic where it is software (the x87 format on x86-64 Linux and macOS,
  binary128 on AArch64 Linux and WebAssembly, `_Float128`): `+ - * /` of normal values
  take integer fast paths, and division is Knuth's algorithm D: a `long double` Horner
  loop 5.8 times faster (x87 format), a division 27 times; binary128 2.3 and 5.5 times.
- x86-64 code generation: a floating-point constant used once is the memory operand of
  a scalar `addsd`/`mulsd` (and the like) without AVX too, and of a commutative VEX
  instruction whichever its place; `@fma(a, b, -c)` is a `vfmsub` and `@fma(-a, b, c)`
  a `vfnmadd`; and a function whose returns meet in one block (an inlined callee's)
  returns from each path, so that the paths that make no call need no stack frame.

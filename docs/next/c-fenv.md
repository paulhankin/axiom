## C

- `#pragma STDC FENV_ACCESS ON` (and the option `--c-fenv-access`) is honoured: in a
  function that has it, the optimizer does not fold, merge, move or remove floating
  operations, and constant initializers of automatic objects are evaluated at run time.
  The relational operators `<`, `<=`, `>` and `>=` raise "invalid" for a NaN operand, as
  Annex F asks (`isless` and the like do not).
- `#pragma STDC FENV_ROUND` (C23) sets the static rounding direction of its scope, at run
  time and at translation time: floating constants in its scope, and the constant
  expressions of static initializers folded there, are rounded in it (F.8.2). The
  functions of C23 7.6.2's table follow it in every format, a wide `long double` and
  `_Float128` included (`sqrtl`, `fmal`, `rintl`, `nearbyintl`, `lrintl`, `scalbnl`,
  `fdiml` read the current direction and raise their exceptions).
- `#pragma STDC FENV_DEC_ROUND` is honoured in every direction (`FE_DEC_TONEARESTFROMZERO`
  too), at translation and at run time, for the decimal operators and the functions of
  7.6.3's table.
- The software floating arithmetic honours the environment in those functions: `long
  double` where it is wider than `double`, `_Float128`, `_Float64x`, `_Float16`'s rounding,
  and the conversions of wide integers and decimal values to binary ones round in the
  current direction and raise IEEE 754's exceptions (tininess after rounding) in the
  hardware's flags. Code without the pragma is as fast as before.
- Annexes F and G are claimed on x86-64 and AArch64 (with the library):
  `__STDC_IEC_559__` and `__STDC_IEC_559_COMPLEX__` are 1, and in C23
  `__STDC_IEC_60559_BFP__` and `__STDC_IEC_60559_COMPLEX__` are `202311L`; `I` is then
  `_Imaginary_I`. `<float.h>` has `FLT_SNAN`, `DBL_SNAN` and `LDBL_SNAN` (C23), and
  `FLT16_SNAN` ... under `__STDC_WANT_IEC_60559_TYPES_EXT__`. Not on WebAssembly, which has
  no floating-point environment.

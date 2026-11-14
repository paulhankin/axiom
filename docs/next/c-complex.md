## C

- Complex types: `float _Complex`, `double _Complex` and `long double _Complex` (and of
  `_Float16` ... `_Float64x`), in every mode from C99, with `<complex.h>` (`I`, `CMPLX`,
  `CMPLXF`, `CMPLXL`, and `creal`, `cimag` and `conj` in line) and the library's complex
  functions of every type (`cexpf128`, `csqrtf16`, ...; C23 Annex H), which `<tgmath.h>`
  chooses for complex arguments. `__STDC_NO_COMPLEX__` is no longer defined. Arithmetic
  keeps a real operand real (Annex G.5), and multiplication and division recover
  infinite and zero results as G.5.1 describes; constants are folded with the same
  algorithms. Complex values are passed, returned and read with `va_arg` as each
  target's C ABI says.
- The imaginary types of Annex G (`float`, `double` and `long double _Imaginary`,
  `imaginary` and `_Imaginary_I` in `<complex.h>`), with G.4's conversions and G.5's
  arithmetic; `2.0i` constants in a GNU dialect.
- In the GNU dialects, `__real__` and `__imag__` give the parts of a complex value (lvalues
  of an lvalue's parts).

## C

- Complex types: `float _Complex`, `double _Complex` and `long double _Complex`, in
  every mode from C99, with `<complex.h>` (`I`, `CMPLX`, `CMPLXF`, `CMPLXL`, and
  `creal`, `cimag` and `conj` in line; the other functions are declared for a
  library). `__STDC_NO_COMPLEX__` is no longer defined. Arithmetic keeps a real operand
  real (Annex G.5), and multiplication and division recover infinite and zero results
  as G.5.1 describes; constants are folded with the same algorithms. Complex values
  are passed, returned and read with `va_arg` as each target's C ABI says. The
  imaginary types remain unsupported (`C3901`).

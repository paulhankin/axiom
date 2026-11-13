## C

- `<complex.h>` has the complex functions of `_Float16` and `_Float128` (`cexpf128`,
  `csqrtf16`, ...; C23 Annex H) and macros for those of `_Float32` ... `_Float64x`;
  `<tgmath.h>` chooses them for complex arguments of those types.
- In the GNU dialects, `__real__` and `__imag__` give the parts of a complex value (lvalues
  of an lvalue's parts).

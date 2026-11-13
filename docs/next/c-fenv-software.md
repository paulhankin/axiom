## C

- The software floating arithmetic honours the floating-point environment under
  `#pragma STDC FENV_ACCESS ON` (and `--c-fenv-access`, and a static `FENV_ROUND`
  direction): `long double` where it is wider than `double`, `_Float128`, `_Float64x`,
  `_Float16`'s rounding, and the conversions of wide integers and decimal values to binary
  ones round in the current direction and raise IEEE 754's exceptions (tininess after
  rounding) in the hardware's flags. Code without the pragma is as fast as before.

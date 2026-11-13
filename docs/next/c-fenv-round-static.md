## C

- `#pragma STDC FENV_ROUND`'s static direction now applies at translation time too:
  floating constants in its scope, and the constant expressions of static initializers
  folded there, are rounded in it (C23 F.8.2). The functions of C23 7.6.2's table follow
  it in every format, a wide `long double` and `_Float128` included (`sqrtl`, `fmal`,
  `rintl`, `nearbyintl`, `lrintl`, `scalbnl`, `fdiml` read the current direction and raise
  their exceptions).
- `#pragma STDC FENV_DEC_ROUND` is honoured in every direction (`FE_DEC_TONEARESTFROMZERO`
  too), at translation and at run time, for the decimal operators and the functions of
  7.6.3's table; it was a warning (C4113) for the directed ones.

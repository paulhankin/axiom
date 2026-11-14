## C

- C23's decimal floating types, `_Decimal32`, `_Decimal64` and `_Decimal128`, and their
  constants (`1.20dd`): IEEE 754-2019's decimal formats in the BID encoding of the
  platform ABIs, correctly rounded with the quanta IEEE 754 prefers, in every decimal
  rounding direction (`fe_dec_setround`), with their exceptions in `<fenv.h>`'s flags,
  on every target; conversions with every integer and binary floating type; the
  calling conventions of x86-64 System V and AAPCS64 (checked against the system's C
  compiler). The library has all of C23's decimal functions of `<math.h>`, `<tgmath.h>`
  dispatch, `strtod64` and its kin (which keep a NaN's payload from its n-char-sequence,
  as `fscanf` does), `fprintf` and `fscanf`'s `H`, `D` and `DD`, `strfromd64`, and the
  `<float.h>` macros; `__STDC_IEC_60559_DFP__` is defined. Under `#pragma STDC
  FENV_ACCESS ON` decimal operations are done at run time, as the binary ones are;
  `iseqsig` takes decimal operands.
- The elementary decimal functions (`expd64`, `logd64`, `sind64`, `powd64`, `atan2d64`,
  `erfd64`, `tgammad64`, the C23 ones ...) are within 1 ulp (0.500 measured), those of
  `_Decimal32` and `_Decimal64` correctly rounded in every decimal rounding direction,
  with the quantum C23 prefers for exact results (`atan2pid64(1, -1)`
  is `0.75`, `exp2d64(3.0dd)` is `8`, `compoundnd64(0.1dd, 2)` is `1.21`): those of
  `_Decimal32` and `_Decimal64` take about 0.5 to 1 us (x86-64, `-O2`), computed in
  binary128 with a rounding test that falls back to the decimal computation; those of
  `_Decimal128` (and `_Decimal64x`) take the same path from the long double functions'
  triple-double values, 2 to 15 us (`powd128` 30).
- The arithmetic takes integer fast paths: a `_Decimal64` multiplication, addition or
  division about 100 ns, `_Decimal128`'s 75 to 200 ns.
- Fixed: `exp2d32` and the other exponentials of a tiny argument trapped, and their
  results in the directed rounding modes were not correctly rounded.

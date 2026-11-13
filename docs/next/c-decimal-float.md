## C

- C23's decimal floating types, `_Decimal32`, `_Decimal64` and `_Decimal128`, and their
  constants (`1.20dd`): IEEE 754-2019's decimal formats in the BID encoding of the
  platform ABIs, correctly rounded with the quanta IEEE 754 prefers, in every decimal
  rounding direction (`fe_dec_setround`), with their exceptions in `<fenv.h>`'s flags,
  on every target; conversions with every integer and binary floating type; the
  calling conventions of x86-64 System V and AAPCS64 (checked against the system's C
  compiler). The library has all of C23's decimal functions of `<math.h>` (the
  elementary ones within 1 ulp), `<tgmath.h>` dispatch, `strtod64` and its kin,
  `fprintf` and `fscanf`'s `H`, `D` and `DD`, `strfromd64`, and the `<float.h>` macros;
  `__STDC_IEC_60559_DFP__` is defined. Under `#pragma STDC FENV_ACCESS ON` decimal
  operations are done at run time, as the binary ones are; `iseqsig` takes decimal
  operands.

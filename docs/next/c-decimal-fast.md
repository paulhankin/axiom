## C library: faster decimal floating point

- `_Decimal32` and `_Decimal64` division takes a 128-by-64-bit integer path (3 to 4
  times faster), and `_Decimal128`'s `+`, `-`, `*` and `/` 128- and 256-bit integer paths
  (2 to 6 times faster); the results and flags are the same bits as before.
- The elementary functions of `_Decimal32` and `_Decimal64` (`expd64`, `logd64`,
  `sind64`, `powd64`, `atan2d64`, `erfd64`, `tgammad64`, the C23 ones ...) compute in
  binary128 with a rounding test, falling back to the decimal computation: 50 to 100
  times faster, and correctly rounded in every decimal rounding direction.
- `exp2d32` and the other exponentials of a tiny argument no longer trap (the working
  precision of ln 2 and ln 10 went negative), and their results in the directed
  rounding modes are now correctly rounded.
- `_Decimal128`'s (and `_Decimal64x`'s) elementary functions take that path too, from
  the long double functions' triple-double values before their rounding: 3 to 13 times
  faster (`expd128` 2.3 us, was 25 to 48; `logd128` 4.3, was 58 to 74; `lgammad128` and
  `tgammad128` 2.3 and 5.5 times), the same results and flags.

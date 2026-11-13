## C library and standard library: faster floating point

- `std/math`'s `atan`, `atan2`, `tan`, `sinh`, `cbrt` and `lgamma` are now about as fast
  as the less accurate functions they replaced (0.9 to 1.15 times their time; they were 1.3
  to 2.1 times), with the same or better accuracy (`atan`, `atan2`, `tan` correctly
  rounded on the measured inputs, `cbrt` within 0.51 ulp).
- The C library's `erf` and `erfc` take about 1.15 times the system library's time
  (were 2.7 and 3.2), `lgamma` 1.4 (2.8), `lgammaf` 1.6 (3.5), `acosh` 1.7 (2.1),
  `asinh` 1.5, `hypot` 1.3, `exp10` 1.3, `powf` 1.6 (2.0); the results are the same
  bits as before.
- `long double` arithmetic where it is software (the x87 format on x86-64 Linux,
  binary128 on AArch64 Linux and WebAssembly, `_Float128`): `+ - * /` of normal values
  take integer fast paths, and division is Knuth's algorithm D: a `long double` Horner
  loop 5.8 times faster (x87 format), a division 27 times; binary128 2.3 and 5.5 times.
  `erfl` and `erfcl` are 3 to 5 times faster.
- `_Decimal32` and `_Decimal64` addition and multiplication take 64-bit integer fast
  paths: a `_Decimal64` multiply-add 5 times faster.

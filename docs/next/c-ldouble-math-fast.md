## C library: faster long double functions

- The x87 format's `long double` functions (x86-64 Linux: `expl`, `logl`, `sinl`,
  `atanl`, `asinl`, `powl`, `hypotl`, `cbrtl`, the hyperbolic and inverse hyperbolic
  functions, `erfl`, `erfcl`, `lgammal`, `tgammal` ...) take a double-double path with a
  rounding test before the triple-double one: 2 to 20 times faster (`log1pl` 5, `asinl`
  9, `erfl` 30, `lgammal` 10), the same bits; most now take 1.3 to 3.4 times the x87
  hardware's functions of the system library, the trigonometric ones, `powl` and
  `tgammal` less.
- binary128's (and `_Float128`'s) `erf` and `erfc` are 5 to 6 times faster (Taylor
  polynomials of the scaled complementary error function by pieces).
- `asinhl` and `acoshl` of arguments beyond about 2^512 were wrong (the square of the
  argument overflowed a double); `powl(x, n)` for an integer n whose exact result fits
  128 bits is now exact and correctly rounded (ties to even).

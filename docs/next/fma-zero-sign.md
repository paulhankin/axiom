## Runtime

- `@fma(a, b, c)` (and C's `fma`) where `c` is a zero and `a * b` rounds to zero now
  gives the zero of the product's sign, as IEEE 754 does: with `x` = -2⁻⁶⁰⁰ and
  `y` = 2⁻⁶⁰⁰, `@fma(x, y, 0.0)` is `-0.0`. It was `+0.0` on WebAssembly and on x86-64
  without the `fma` extension, which compute `@fma` of f64 in software. A NaN, an
  infinity or a nonzero result was right.

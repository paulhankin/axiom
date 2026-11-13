## C library: faster double complex functions

- `cexp`, `csqrt`, `csinh`, `ccosh`, `ctanh`, `casin`, `cacos`, `catanh` (and `csin`,
  `ccos`, `ctan`, `casinh`, `cacosh`, `catan`) take 0.75 to 0.85 times their former
  time; the results are the same bits.
- Fixed: on a processor without fused multiply-add, `catanh` and `catan` of an argument
  near 1e150 with a very small other part returned a NaN part, and `casin` and `casinh`
  of an argument with a subnormal part could differ in its last bit from the result with
  fused multiply-add.

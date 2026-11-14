## C

- `sinf(-0.0f)` is `-0.0f` (it was `+0.0f`), as Annex F asks; the Axiom standard
  library's float sine shares the fix.
- Division of `float _Complex` values is computed in `double`, so that parts near the
  ends of float's range (a subnormal one) no longer lose the result's precision.
- More GNU `__builtin_` forms of C library functions: `fprintf`, `fputs`, `fputc`,
  `putc`, `fwrite`, `scanf`, `sscanf`, `fscanf`, `strcspn`, `strspn`, `strpbrk`,
  `imaxabs`.
- The optimizer folds comparisons with a NaN constant (an ordered one is false, `!=`
  true, also of a const object holding a NaN) and `x > +inf`, `x < -inf`.

## Compiler

- On x86-64 with AVX-512 (`--target=native` on such a processor, `--cpu=x86-64-v4`, or
  an `@features` function), a function that converted an `f32x16` to two integer
  vectors with `@int_from_float` (to `i8x16` and to `u16x16`, say) could stop the
  compiler with "internal compiler error: xmm16-31 in a VEX instruction": a scalar
  move of a lane in one of the registers only AVX-512 has. Those moves are now encoded
  for them.

## Compiler

- On x86-64, `@narrow` of 16- and 32-bit lanes is a pack instruction (`packsswb`,
  `packuswb`, `packssdw`, `packusdw`; with a `vpermq` at 256 and 512 bits, and a
  `pminuw` or `pminud` first for unsigned lanes) instead of lane by lane:
  `@narrow(u8x64, a, b)` of `i16x32` is three instructions with AVX-512 where it was
  several hundred. Unsigned lanes and `packusdw` need SSE4.1; without it those are
  still lane by lane.

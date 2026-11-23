## Compiler

- On x86-64 with AVX-512, shifting a 512-bit vector of 32-bit lanes by a lane of
  another vector (`a << @extract_lane(k, 3)` with `a: i32x16`, `k: u32x8`) no longer
  crashes the compiler.

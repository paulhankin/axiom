## Compiler

- Loops that copy or fill consecutive elements become `memmove`, `memcpy` (C's
  `restrict`, or two distinct objects) and `memset` at `-O1` and above; on x86-64 such
  operations of a length unknown until run time take inline paths for up to 32 bytes.
- At `-O2` and `-O3`, counted loops of element-wise arithmetic and integer reductions over
  arrays run as vector loops (x86-64 SSE2, or AVX2 where the function may use it; AArch64
  Advanced SIMD), with a runtime check for a store into the elements a load still reads,
  and searches for an element run first over aligned 16-byte vectors. Floating-point
  reductions are not reordered.
- Switches whose cases each give a constant are table lookups at `-O2`.
- C's `restrict` on pointer parameters is used by the alias analysis.
- x86-64: sign extensions are folded into loads (`movslq` from memory); scalar floats are
  spilled as 8 bytes (also on AArch64).
- Fixed: x86-64 code generation crashed on a sum of a carry and a chained sum.

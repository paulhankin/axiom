## C

- On AArch64, `axiom cc` defines the GNU compilers' built-in names of the Advanced SIMD
  types (`__Float32x4_t`, `__Int8x16_t` and the rest), so the system's `<math.h>`, whose
  vector declarations use them, compiles. It failed with an error in
  `bits/math-vector.h`.

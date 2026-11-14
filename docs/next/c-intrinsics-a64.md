## C

- The AArch64 intrinsics headers `<arm_neon.h>`, `<arm_fp16.h>`, `<arm_bf16.h>` and
  `<arm_acle.h>`: all of Advanced SIMD, with FP16, BF16, FHM, RDM, the dot products and
  i8mm, and AES, SHA-1, SHA-256, SHA-512, SHA-3, SM3, SM4 and CRC32, in every mode
  (docs/c.md, "Intrinsics"). Each intrinsic compiles to its instruction (on 128-bit
  vectors through the IR's vector operations where there is one); one whose extension
  the calling function is not compiled for (`--features`, the attribute `target`) is
  error `C3240`, and a lane or shift count that is not a constant in its range `C3241`.
- A function of internal linkage that is always inlined and never named is no longer
  generated, so including an intrinsics header costs only its parsing.

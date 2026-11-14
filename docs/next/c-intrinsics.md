## C

- The processors' intrinsics headers, in every mode (ISO standards too) and with or
  without the C library (docs/c.md, "Intrinsics"):
  - x86-64: `<mmintrin.h>`, `<xmmintrin.h>`, `<emmintrin.h>`, `<pmmintrin.h>`,
    `<tmmintrin.h>`, `<smmintrin.h>`, `<nmmintrin.h>`, `<wmmintrin.h>`, `<immintrin.h>`,
    `<x86intrin.h>` and `<cpuid.h>`: MMX (computed with SSE2) to AVX2, FMA, F16C, BMI1,
    BMI2, LZCNT, POPCNT, ADX, AES, PCLMULQDQ, SHA, RDRAND and RDSEED;
  - AArch64: `<arm_neon.h>`, `<arm_fp16.h>`, `<arm_bf16.h>` and `<arm_acle.h>`: all of
    Advanced SIMD, with FP16, BF16, FHM, RDM, the dot products and i8mm, and AES, SHA-1,
    SHA-256, SHA-512, SHA-3, SM3, SM4 and CRC32;
  - WebAssembly: `<wasm_simd128.h>`, simd128 and relaxed-simd (`v128_t`,
    `wasm_i32x4_add`, `wasm_i8x16_shuffle`, `wasm_f32x4_relaxed_madd` ...).
- Each intrinsic compiles to its instruction, through the IR's vector operations where
  there is one (so the optimizer sees through it). One whose extension the calling
  function is not compiled for (`--features`, the attribute `target`) is error `C3240`;
  a lane, shift count or shuffle index that is not a constant in its range is `C3241`.
- Not provided: the AVX-512, AMX and Key Locker intrinsics, `<arm_sve.h>`, and a few
  AArch64 extensions' (docs/c.md, "Known gaps").
- A function of internal linkage that is always inlined and never named is no longer
  generated, so including an intrinsics header costs only its parsing.

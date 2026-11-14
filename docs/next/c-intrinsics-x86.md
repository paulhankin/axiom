## C

- The x86-64 intrinsics headers: `<mmintrin.h>`, `<xmmintrin.h>`, `<emmintrin.h>`,
  `<pmmintrin.h>`, `<tmmintrin.h>`, `<smmintrin.h>`, `<nmmintrin.h>`, `<wmmintrin.h>`,
  `<immintrin.h>`, `<x86intrin.h>` and `<cpuid.h>`: MMX (computed with SSE2) to AVX2,
  FMA, F16C, BMI1, BMI2, LZCNT, POPCNT, ADX, AES, PCLMULQDQ, SHA, RDRAND and RDSEED, in
  every mode. Each intrinsic compiles to its instruction, through the IR's vector
  operations where there is one; one whose extension the calling function is not
  compiled for (`--features`, the attribute `target`) is error `C3240`. Programs that
  needed `-U__SSE2__` to build no longer do (docs/c.md, "Intrinsics").

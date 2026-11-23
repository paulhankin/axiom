## Compiler

- On x86-64 processors with AVX-512 VNNI but not AVX-VNNI, `@dot` of `u8` by `i8` lanes on
  128- and 256-bit vectors is `vpdpbusd` (its EVEX form) instead of lane by lane; where
  both are present the EVEX form is used at every width.

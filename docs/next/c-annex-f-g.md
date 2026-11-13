## C

- Annexes F and G are claimed on x86-64 and AArch64 (with the library):
  `__STDC_IEC_559__` and `__STDC_IEC_559_COMPLEX__` are 1, and in C23
  `__STDC_IEC_60559_BFP__` and `__STDC_IEC_60559_COMPLEX__` are `202311L`; `I` is then
  `_Imaginary_I`. `<float.h>` has `FLT_SNAN`, `DBL_SNAN` and `LDBL_SNAN` (C23), and
  `FLT16_SNAN` ... under `__STDC_WANT_IEC_60559_TYPES_EXT__`. Not on WebAssembly, which has
  no floating-point environment.

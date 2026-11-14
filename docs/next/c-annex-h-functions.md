## C library

- `<math.h>` declares every function of C23 Annex H.11.2 for `_Float16`, `_Float32`,
  `_Float64`, `_Float128`, `_Float32x` and `_Float64x` (with
  `__STDC_WANT_IEC_60559_TYPES_EXT__`): those of `_Float32` ... `_Float64x` are now
  functions whose results have those types (they were macros for the standard
  functions), C23's additions exist for `_Float16` and `_Float128` too (`exp2m1f128`,
  `rootnf16`, `fromfpf128`, `totalorderf16`, `canonicalizef128`, `getpayloadf16`, ...,
  `nanf16`, `nanf128`), and so do the 90 narrowing operations between the types
  (`f16addf32`, `f32mulf64x`, `f64xsqrtf128` ...: exact and rounded once).
- H.11.3's encoding functions (`encodef16`, `decodef128`, `f32encf16`, `d32encdecd64`
  ...), `strtofN`, `strfromfN` and `wcstofN` for every type, the complex functions of
  `_Float32` ... `_Float64x`, and `long_double_t`, `_Float16_t` ... `_Float128_t`,
  `_Decimal32_t` ... `_Decimal128_t`. `<tgmath.h>` chooses each type's own function.
- `encodedecd32` ... `decodebind128` take the encoding as an array of `unsigned char`
  (C23 7.12.16), no longer as a decimal object: a program that passed a `_Decimal64 *`
  passes an `unsigned char` array of 8.

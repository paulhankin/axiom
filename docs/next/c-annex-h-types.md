## C: Annex H claimed

- C23 mode defines `__STDC_IEC_60559_TYPES__` (`202311L`) with the library (hosted, not
  `--c-lib=system`), on every target.
- `_Decimal64x` (Annex H.2.4), in decimal128's format: a type of its own, with the
  constant suffixes `d64x` and `D64x` (and H.5.2's `d32`, `d64`, `d128`), H.4.3's usual
  arithmetic conversions (`_Decimal64 + _Decimal64x` is `_Decimal64x`,
  `_Decimal64x + _Decimal128` is `_Decimal128`), `<float.h>`'s `DEC64X_` macros,
  `HUGE_VAL_D64X`, every `d64x` function of `<math.h>`, the narrowing operations to and
  from it (`d32addd64x`, `d64xaddd128` ...), `strtod64x`, `strfromd64x` and `wcstod64x`.
- H.12.4 and H.12.5's string conversions of encodings: `strfromencf16` ...
  `strfromencf128`, `strtoencfN`, `wcstoencfN`, and `strfromencdecdN`,
  `strfromencbindN`, `strtoencdecdN`, `strtoencbindN`, `wcstoencdecdN`,
  `wcstoencbindN` for decimal32 ... decimal128.
- `<tgmath.h>` has the narrowing macros `d32add` ... `d64sqrt` (C23 7.27p11) and, with
  `__STDC_WANT_IEC_60559_TYPES_EXT__`, `f16add` ... `f64xsqrt` and `d64xadd` ... (H.13p5),
  and `quantize`, `samequantum`, `quantum`, `llquantexp`; an integer argument counts as
  `_Float32x` beside a binary extended argument (`pow(f32x, 2)` calls `powf32x`) and as
  `_Decimal64x` beside a `_Decimal64x` one (H.13p3).
- The static directions of `#pragma STDC FENV_ROUND` and `FENV_DEC_ROUND` now apply to the
  narrowing operations (`f32addf64`, `d32addd64` ...), the encoding conversions
  (`f32encf64`, `d32encbind64` ...) and the string conversions of encodings.

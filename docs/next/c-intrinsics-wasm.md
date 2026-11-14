## C

- `<wasm_simd128.h>`: the WebAssembly vector intrinsics of simd128 and relaxed-simd
  (`v128_t`, `wasm_i32x4_add`, `wasm_i8x16_shuffle`, `wasm_f32x4_relaxed_madd` ...), in
  every mode from C99 (docs/c.md, "Intrinsics"). A relaxed-simd intrinsic called from a
  function not compiled for relaxed-simd is error `C3240`.

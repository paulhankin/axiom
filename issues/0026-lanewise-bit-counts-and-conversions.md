# 0026: Lane-wise @clz/@ctz/@popcount and int/float conversions on vectors

- Type: feature
- Status: open
- Area: language, compiler (C and selfhost)

## Today

`@clz`, `@ctz` and `@popcount` take scalars only, and `@float_from_int` and
`@int_from_float` reject vectors (`E0822 @float_from_int requires a float
target type`). Yet WebAssembly has the conversions (`f32x4.convert_i32x4_s/u`,
`i32x4.trunc_sat_f32x4_s/u`) and `i8x16.popcnt`, and x86 has `vplzcntd/q`
(AVX-512CD), `vpopcntb/w/d/q` (AVX-512 BITALG/VPOPCNTDQ) and the AVX-512
conversions, so these are omissions rather than portability limits.

the benchmark's per-lane hitting-shot code (issue 0025) takes "the highest point
that can hit" per lane as `31 - lzcnt(mask)`, and counts rolls with
`popcount` of a lane mask. Without vector `@clz` it has to be built from
five compare-and-shift steps per vector, and without conversions the usual
float-exponent tricks for powers of two and leading zeros are out of reach
too.

## Proposal

- `@clz(v)`, `@ctz(v)`, `@popcount(v)` lane-wise on integer vectors, the
  result of the same type (as for scalars).
- `@float_from_int(F, v)` and `@int_from_float(I, v)` lane-wise between
  vectors of equal lane count, with the scalar rules per lane (round to
  nearest; truncate and saturate, NaN to 0, as `trunc_sat`).
- Lowering: wasm32 has the conversions and `i8x16.popcnt` (wider popcounts
  by adding pairs); `@clz`/`@ctz` there and on SSE2/AVX2 by the
  float-exponent method or a nibble `pshufb` table; AVX-512 has single
  instructions for all of them; NEON has `clz`, `cnt`, `scvtf`, `fcvtzs`.

# 0027: Loads and stores of a vector's first lanes (masked tails)

- Type: feature
- Status: open
- Area: language, compiler (C and selfhost)

## Today

`@load(V, s)` traps when `s` is shorter than the vector (T0006), so a loop
over vectors needs a separate scalar loop for its tail. x86 loads and stores
a vector's first lanes without touching the memory past them (AVX-512 masked
moves, AVX2 `vmaskmovps`), and every Rust kernel in the benchmark's
`simd_avx512.rs` reads its last, partial block of inputs with
`_mm512_maskz_loadu_ps(tail_mask, ptr)`: 250 inputs are 15 full blocks and
one of 10.

## Proposal

- `@load_partial(V, s)`: lanes `0..min(s.len, N)` from `s`, the rest zero.
- `@store_partial(s, v)`: writes lanes `0..min(s.len, N)`.

Both never touch memory beyond `s`. Lowering: AVX-512 masked moves (mask
`(1 << n) - 1`), AVX2 `vmaskmovps/d` for 32-bit lanes, elsewhere a full load
when the slice is known long enough and lane-by-lane otherwise.

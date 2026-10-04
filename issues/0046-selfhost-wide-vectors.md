# 0046: Self-hosted x86-64 backend: 512-bit vector code is about 10x slower than LLVM's

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost, x86-64 backend)

## Today

With issue 0039 fixed, the self-hosted compiler builds the benchmark's Axiom port natively
and its results are bit-identical to the C compiler's. But on an AVX-512 host its kernels
are an order of magnitude slower, slower even than its own wasm build (ns per call, -O2,
the benchmark `axiom/src/bin/bench_kernels.ax`):

| kernel | C compiler native | self-hosted native | self-hosted wasm |
|---|---|---|---|
| contact_inputs | 224 | 2094 | 1079 |
| contact_net | 343 | 3534 | 1178 |
| race_net | 197 | 3216 | 816 |
| pruned_contact | 89 | 750 | 348 |
| pruned_boards | 54 | 616 | 188 |

Operations on 256- and 512-bit vectors that the instruction selector does not take whole
are split into 128-bit pieces by `vsplit`, so the code is full of
`vextracti32x4`/`vpshufb`/`vinserti32x4`, and vector float-to-int conversions go lane by
lane: `simd_avx512.dim16` is about 4,200 instructions against the C compiler's 340.

## Proposal

Select whole-width instructions for what the target has (with AVX-512F/BW/DQ/VL and FMA:
loads and stores incl. masked `@load_partial`/`@store_partial`, arithmetic and `@fma`,
compares into mask registers and `@select` as blends, `@bitmask` via `vpmovd2m`/`kmov`,
shuffles/`@swizzle`/`@swizzle2` as `vpermd`/`vpermt2d`/`vpermps`, `@int_from_float` as
`vcvttps2dq`/`udq` with the clamp sequence, `@float_from_int`, `@reduce_add` halves,
`@dot` as `vpmaddwd`, per-lane shifts `vpsllvd`/`vpsrlvd`, `@clz`/`@popcount` as
`vplzcntd`/`vpopcnt*`), and keep `vsplit` for targets that lack them. A per-instruction
count test like the C compiler's `tests/lowering/` would keep it honest.

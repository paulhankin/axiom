# 0046: Self-hosted x86-64 backend: 512-bit vector code is about 10x slower than LLVM's

- Type: feature (performance)
- Status: fixed (1c3122c, 99e3fbe, 6015b69, 71902bd; test 1942bd0)
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

## Resolution

The self-hosted x86-64 backend now selects whole-width AVX2/AVX-512 instructions for what
the function's extensions have, and `vsplit` only splits what they lack:

- 1c3122c: `@swizzle`/`@swizzle2` as `vpermd`/`vperm*`/`vpermt2*` (zeroing through a
  `vpcmpu*` mask unless the indices are masked constants), other wide shuffles as one
  permute by constant indices, vector `@int_from_float`/`@float_from_int` as `cvt*`
  (with the saturation fixups), `@load_partial`/`@store_partial` as masked moves,
  per-lane shifts `vpsllv`/`vpsrlv`/`vpsrav`, `@clz`/`@popcount` as
  `vplzcnt`/`vpopcnt`; the EVEX encoder takes every vector length and masks.
- 99e3fbe: 512-bit values also use zmm16-31; constant splats and loads of constant
  globals are rematerialized at their uses; phi copies without a temporary each;
  `pxor r, r` no longer reads `r`.
- 6015b69: constant `@vec` folds to a constant, integer addresses become `ptradd`
  (addressing modes), the partial moves are memory accesses to GVN/DSE and the
  shadow-stack pass.
- 71902bd: `@select` as one `vpternlogd`, `@bitmask` of a compare straight from `k1`,
  folded vector loads and constants as memory operands.
- 1942bd0: `tests/lowering/avx512_whole.ax`, checked for the self-hosted compiler
  through `objdump` of its object file.

the benchmark `axiom/src/bin/bench_kernels.ax`, ns per call, `--target=native -O2`, AVX-512
host (min of 5 runs on a loaded machine); checksums identical for all three, and
`bench_search --mode 2ply --limit 20` RESULTs identical to the C compiler's:

| kernel | C compiler | self-hosted before | self-hosted after |
|---|---|---|---|
| contact_inputs | 175 | 1890 | 326 |
| contact_net | 227 | 3364 | 301 |
| race_net | 157 | 3152 | 229 |
| contact_single | 239 | 3198 | 291 |
| pruned_contact | 77 | 680 | 100 |
| pruned_race | 105 | 922 | 128 |
| pruned_single | 106 | 851 | 145 |
| pruned_boards | 44 | 551 | 70 |
| race_inputs | 92 | 198 | 197 |
| pruning_inputs | 16 | 36 | 34 |
| movegen | 650 | 986 | 994 |

What remains is mostly scalar code quality (`movegen`, `race_inputs`,
`pruning_inputs`, and much of `contact_inputs`, which the vector work did not
touch), plus smaller vector items: broadcasts from memory (`vbroadcastss m`),
`@select` through `k` masks (`vblendm*`) instead of expanded lanes, and `@any_true`/
`@all_true` of compares via `vptestm`.

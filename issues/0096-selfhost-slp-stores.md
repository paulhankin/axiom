# 0096: Self-hosted x86-64: neighbouring stores of computed values not vectorized (the benchmark's race inputs)

- Type: feature (performance)
- Status: fixed (fbe0dc3), except the two-lane `vplzcntd` (left in 0094)
- Area: compiler (selfhost: slp, thread, x64)

Follow-up to 0078 (the SLP of compare masks, `slp.ax`) and item 3 of 0094. Measured on
the benchmark 1d00a0e (`axiom/src/bin/bench_kernels.ax`), Xeon @ 2.80GHz (Cascade Lake).

Since the benchmark bc2f24b, `calculate_race_inputs` (`axiom/src/eval/features.ax`) stores
every race input, as the Rust engine does, a 0 or a 1 rather than a branch:

```
inputs[k] = select(nc == 1, 1.0, 0.0);      // one
inputs[k + 1] = select(nc == 2, 1.0, 0.0);  // two
inputs[k + 2] = select(nc >= 3, 1.0, 0.0);  // three
inputs[k + 3] = select(nc > 3, (nc - 3) / 2.0, 0.0);
...
inputs[base + RI_OFF + k] = select(men_off == k + 1, 1.0, 0.0);  // k in 0..14
```

LLVM's SLP vectorizer makes each group of neighbouring stores one vector store: a
broadcast of `nc` (or `men_off`), a compare against a constant vector into a mask
register and a zero-masked move of `1.0` (`vpcmpeqb k1; vmovaps ymm{k1}{z}`), the fourth
lane inserted (`vinsertps`). The self-hosted compiler stores each lane on its own, and
jump threading first turns the four tests into a decision tree (each test's outcome
decides the next: 0063), so `one` .. `more` are phis of five-way joins rather than
selects. Per 300 calls (single-stepped): 356k instructions and 30k taken branches,
against LLVM's 305k and 8k; `race_inputs` 137 ns against 93 (Rust 110).

Also from 0094 (to take if they fit):

- `calculate_half_inputs_with_masks`: LLVM computes the two back anchors'
  `highest_point` with one two-lane `vplzcntd` (0078 noted it).
- `dim128`'s eight threshold loads (`@load(f32x16, ht[16 * k..])` after
  `@expect(ht.len >= 128)`) each keep two compares where LLVM keeps one range check:
  about 40 instructions per evaluation (`contact_single` 430 ns against LLVM's 397).

## Proposal

- `slp.ax`: vectorize runs of stores to neighbouring elements bottom-up, as LLVM's SLP
  vectorizer does: lanes that are the same operation (selects, compares, arithmetic,
  conversions, neighbouring loads) become one vector operation; constants, a value in
  every lane and other scalars are put together (constant vector, splat, inserts);
  taken when the vector code is cheaper than the scalar code it makes dead. Compares
  of one value against constants with different predicates are one unsigned compare of
  `x - lo` against `hi - lo` per lane.
- `thread.ax`: jump threading should not thread through the join of a branch that
  if-conversion turns into selects (the race inputs' four tests).
- x64: `insertps` for float lanes (now through memory).

## Fix (fbe0dc3)

- `slp.ax` (`store_groups`, after the scalar passes at -O1 and up, x86-64): runs of
  stores to neighbouring elements of a block are one vector store of 32 bytes (AVX2),
  16, or 8 (the low half of a register), built bottom-up: selects of i1 conditions as
  a lane mask (a zero-masked move with AVX-512), compares of one value against
  constants (with different predicates: `x - lo < hi - lo + 1` per lane, unsigned),
  same compares of other lanes, arithmetic, `sitofp`/`bitcast` of lanes as wide,
  neighbouring loads read before the stores; constants, splats and inserted scalars
  otherwise. Taken when the vector code is cheaper than the scalar code it makes dead.
  The race inputs are now `vpbroadcastd; vpsubd; vpcmpltud k1; vinsertps;
  vmovdqu32{k1}{z}; vmovdqu` per point.
- `thread.ax`: the threading item was done in 0753e8d (0097); here only conversions
  between floats and integers became speculatable.
- x64: `insertps` (SSE4.1) or two `shufps` for float lanes instead of a trip through
  memory; AVX-512 integer broadcasts from a general register.
- 0094 item 4 (dim128's threshold loads): `gvn.ax` keeps lower bounds as facts (`k <
  x`, `x >= k`, ranges) and `x - d` is at least `k` when `x` is at least `k + d`; and
  `@load`/`@store` check the length as a count of elements (`len >= ceil(bytes /
  size)`, both compilers) instead of `len * size >= bytes`. dim128 keeps only the
  `@expect` test before its eight loads (`tests/lowering/x64_lower_bounds.ax`).

bench_kernels (min of 9, Cascade Lake, loaded machine), base 639709e / this / LLVM / Rust:
`race_inputs` 109.3 / 69.9 / 93.5 / 108.4 ns; `contact_single` 429 / 427 / 397 / 437
(within noise); others unchanged. SIMD-off (`--cpu=x86-64`): `race_inputs` 144 -> 137
(the float selects stay branches there, so little is vectorized). 2-ply search results
bit-identical.

Not done: the two back anchors' `highest_point` as one two-lane `vplzcntd` (an SLP of
scalar values not feeding stores; small).

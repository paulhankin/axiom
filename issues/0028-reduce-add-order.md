# 0028: @reduce_add's fixed order: fold the halves

- Type: feature (design)
- Status: open
- Area: language (spec §13.2), compiler (C and selfhost)

## Today

§13.2 fixes float `@reduce_add` as pairwise over neighbours: for 4 lanes
`(l0 + l1) + (l2 + l3)`, and for more "the sum of each half, added", so for
16 lanes `((l0 + l1) + (l2 + l3)) + ...`. Deterministic, which is the point,
but it is not the order hardware-minded code uses. Intel's
`_mm512_reduce_add_ps` (and Rust's `core::arch` version, checked bit for bit
on 100,000 random vectors), `_mm256` hand reductions and most SIMD libraries
fold the high half onto the low: lane i plus lane i + 8, then + 4, + 2, + 1.
That is also the cheapest sequence on any machine with wide registers
(extract the high half, add; no horizontal adds).

Porting such code bit for bit (the benchmark's AVX-512 kernels give Rust's native
results exactly) needs a hand-written reduction of three `@shuffle`s and
adds instead of `@reduce_add`.

## Proposal

Either change the specified order to halves-first (lane i plus lane
i + n/2, recursively), which keeps determinism and lowers to the shortest
sequences everywhere (wasm32 included: two shuffles and two adds for
`f32x4`, as today); or keep it and add `@reduce_add_halves`. The first
changes the results of existing programs by rounding, so it needs a note in
the changelog.

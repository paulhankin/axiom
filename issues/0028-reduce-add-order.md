# 0028: @reduce_add's fixed order: fold the halves

- Type: feature (design)
- Status: fixed (107a199)
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

## Resolution

The order is now halves-first (spec §13.2, Appendix C17): lane i plus lane i + n/2 until one lane is left, the order of `_mm512_reduce_add_ps`. Both compilers build it from vector adds of the halves (the self-hosted one down to 128 bits, then lane by lane in the same order). Float results of `@reduce_add` and `@dot` change by rounding; tests/run/simd.ax and simd_wide.ax have cases that tell the orders apart. Fixed on the way (self-hosted compiler, vsplit): a program's shuffle that looked like vsplit's own part-of-a-wider-value shuffle was kept whole on targets without wide registers (invalid wasm), and an instruction whose result is one piece wide did not hand that piece to its users.

# 0088: Self-hosted register allocator: mask registers and what is left after 0077

- Type: feature (performance)
- Status: fixed in part (14c2504); the rest in 0094
- Area: compiler (selfhost: mach, x64)

Follow-up to 0077.

1. **One AVX-512 mask register.** Every compare goes to `k1` and each reader compares
   again (x64.ax `cmp_to_k`, flag 32). Measured on the contact half-inputs
   (`calculate_half_inputs_with_masks`, 300 calls): the self-hosted code writes a mask
   register 17.9k times against LLVM's 12.6k, but only about 560 of those are the same
   compare repeated with its operands unchanged in a straight line (`vptestmd k1,zmm25,zmm25`
   twice in `hitting_shots_not_on_bar_avx512`); the rest of the difference is the shape of
   the code (0078). A fifth register class (k2-k7 allocatable, `kmovq` for spills and
   copies, the mask as a fifth operand of masked instructions whose `aaa` the encoder takes
   from it) would let a compare be made once per block; expect about 0.1% of that kernel.
   Best done after 0078 lands (it changes the same code).
2. **Rematerialized values rebuilt per block.** A value rematerialized rather than kept in
   a register (a stack argument, a hoisted constant) is rebuilt in each block that uses it
   (`spill_remat`): `af_input` in the half-inputs is loaded 9 times per call (5.4k of 487k
   instructions; LLVM keeps it in `r12`). Its interval spans the function, so its weight
   per position is the lowest; splitting it where pressure is low (a region split like
   `split`'s, with the rebuild as the reload and no stores) would keep it in a register
   through the scalar tail.
3. **Splits at loop edges, loop rotation** (0077 items 3 and "also seen"): not seen in the
   kernels after 0081; loop-invariant loads at a loop's head are not hoisted (needs
   rotation and alias information).

## Resolution

Fixed in part in 14c2504 (measured on the benchmark 1d00a0e, axiom 53203b9 as the base,
deterministic counts by single-stepping, timings min of 11 interleaved runs on the
shared 4-core Xeon, about ±5%):

- **Rematerialized values split like other values** (item 2, `mach.ax` `split`,
  `rewrite_spills`): a stack argument or hoisted constant that loses its register is
  split around calls and loops and between the stretches of code that use it, like any
  other value, each piece built again where it is entered (a clone of the definition,
  `sreload`) and never stored; only what still loses is rebuilt per block and then per
  use as before. A piece read once (outside a loop) is not made: the read takes the
  value from memory or builds it right there. The uses of a rematerialized value are
  weighed by what a register saves (`remat_uses`: nothing at a copy, which can be
  built into the copy's register, or at an instruction that can read it from memory
  outside loops). `af_input` in `calculate_half_inputs_with_masks` is loaded once per
  call instead of nine times; stack reads there 9.5k -> 6.6k, instructions 453k ->
  450k (LLVM 448k) per 300 `contact_inputs`.
- **Operations common to a phi's inputs** (`combine.ax` `sink_phi_ops`):
  `phi(op(x1, c), op(x2, c))` is `op(phi(x1, x2), c)` after the join when each input is
  used only there. `dim16_x8`'s full/partial arms (`m_k = @bitmask(load != 0)` vs
  `@bitmask(load_partial != 0)`) now only load; the eight masks are made once after
  the join, so eight vectors cross the join instead of eight general-purpose masks.
  With it, a `@bitmask` of a compare is moved to its use (`x64.ax` `sinkable`), so
  the or-chain of the masks takes each as it is made (one register, not eight).
  This is what the mask registers would have bought there (LLVM: `korw` of the
  compares).
- **Two-address commuted** (`x64.ax` `gen_alu`): the operand whose life ends at a
  commutative integer operation is its destination (`tzcnt r; add r, base` rather
  than `mov t, base; add t, r`), unless the other is a phi.

| | base | after | LLVM |
|---|---|---|---|
| `dim16_x8` (60 batches, pruned_contact) | 448k insts, 70.5k stack loads | 415k, 59.2k | (batch 381k) |
| `dim128_pair` (16 batches, contact_net) | 711k | 695k | (batch 659k) |
| `dim128_pair` (race_net) | 468k | 459k | |
| `calculate_half_inputs_with_masks` (300) | 453k | 450k | 448k |
| `generate_moves_sub` (100 movegen) | 548k | 546k | 546k |
| `pruned_boards16` (one pass) | 10.59M | 10.49M | 10.37M |

| kernel (ns) | base | after | LLVM | Rust |
|---|---|---|---|---|
| movegen | 811 | 802 | 894 | 837 |
| contact_inputs | 209 | 211 | 207 | 204 |
| pruned_contact | 153 | 147 | 162 | 156 |
| pruned_race | 177 | 170 | 178 | 180 |
| contact_net | 376 | 378 | 366 | 361 |
| race_net | 287 | 293 | 272 | 265 |
| pruned_boards | 60.0 | 59.2 | 58.2 | 54.1 |

The other kernels moved within the noise. `bench_search` (0-2 ply, rollout) results
are bit-identical. Tests: `tests/run/joins.ax`, `tests/lowering/x64_phi_ops.ax`.

Not done (0094):

1. **Mask registers as a register class** (item 1). Counted again: of the 16.1k
   compares into `k1` per 300 contact half-inputs, 557 repeat an earlier one in
   straight-line code (0.12% of the function); in the net kernels none do. The one
   place two masks were live together (`dim16_x8`'s or of eight masks) is handled by
   moving the mask to its use above. Not worth a fifth register class now.
2. **Loops**: hoisting the reloads of a slot a loop never stores into a register free
   all through the loop, after allocation, was tried (it removes 10k of `dim16_x8`'s
   59k stack loads) and measured 9-25% slower on `pruned_contact` in repeated
   interleaved runs, for reasons not found (the loop runs about twice per entry and
   the kernel is bound by loading its inputs; code placement alone moves it by 10%,
   see 0094). Loop rotation was not attempted.
3. LLVM's two-lane `vplzcntd` for the back anchors (0078): not done (straight-line
   vectorization, `slp.ax`).

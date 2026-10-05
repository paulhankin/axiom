# 0088: Self-hosted register allocator: mask registers and what is left after 0077

- Type: feature (performance)
- Status: open
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

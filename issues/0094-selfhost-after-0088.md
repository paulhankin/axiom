# 0094: Self-hosted x86-64: what is left after 0088 (code placement, reloads in loops, race inputs, bounds checks)

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: x64, mach, gvn, slp)

Follow-up to 0088, measured on the benchmark 1d00a0e (`axiom/src/bin/bench_kernels.ax`).

1. **Code placement moves the kernels by up to 13%.** Padding every function's start
   by 1, 16, 32 or 48 bytes (an experiment, not committed) moved `pruned_contact`
   between 136 and 154 ns (and between 159 and 168 with the change of item 2). The
   self-hosted backend aligns functions to 16 bytes and nothing inside them: no loop
   header alignment, no care for jumps that cross or end on a 32-byte boundary (the
   Skylake-family JCC erratum keeps those out of the decoded-µop cache; Cascade Lake is
   one). `dim16_x8`'s inner-loop `je` crossed one in one build and not the other. Kernel
   comparisons under ±10% are suspect until the hot loops are placed deliberately (LLVM
   aligns loop headers to 16 bytes). Branch relaxation (`relax_jumps`) assumes distances
   only shrink, which padding breaks: it needs the padding in its model.
2. **Reloads in loops** (0088 item "loops"). After allocation, `dim16_x8`'s inner loop
   reloads nine values per iteration (59k stack loads per 60 batches; LLVM 21k) while
   rbx and r12-r15 are free all through it: the pieces lost their registers in an
   earlier round, while other values still held them, and splitting is never undone.
   Hoisting those reloads in front of the loop after allocation (tried while fixing 0088:
   a slot never stored in the loop reloaded on the way in, into a register nothing in
   the loop uses) removed 10k loads but measured slower (see item 1); a
   final allocation round that lets split pieces of one slot inside a loop join again
   may do better.
3. **Race inputs**: `calculate_race_inputs` (300 calls) is 356k instructions against
   LLVM's 305k, and `race_inputs` 137 ns against 93 (Rust 110) since the benchmark stores
   the race inputs unconditionally: LLVM vectorizes the stores and compares (29k mask
   writes), the self-hosted compiler does not.
4. **Bounds checks at `dim128`'s start**: the eight threshold loads
   (`@load(f32x16, ht[16 * k..])`, after `@expect(ht.len >= 128)`) each keep two
   compares; LLVM keeps one range check. About 40 instructions per evaluation
   (`contact_single`: 558 instructions per call outside the main loop against 436).
5. A stack argument reload whose register is written again before it is read survives
   allocation (`mov rcx,[rbp+0x10]; mov rcx,[rbp+0x18]` at the start of the contact
   half-inputs): `drop_dead_reloads` runs before allocation only.

# 0077: Self-hosted register allocator: what is left after 0070 (movegen, mask registers, loop splits)

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: mach, x64)

Follow-up to 0070. After it, the benchmark's move generator (`src/core/moves.ax`, 100
`generate_moves_into` calls in `axiom/src/bin/bench_kernels.ax`, counted by single-stepping)
executes 1017k instructions against LLVM's 901k: `generate_moves_sub` 595k (546k),
`save_moves` 229k (226k), and 37k taken branches against 32k in `generate_moves_sub`. What
is left is mostly in the register allocator:

1. **Call arguments shuffled through temporaries** (0070 item 2, not done): the
   `save_moves` call still moves its arguments through registers that are themselves
   arguments.
2. **Stores repeated at joins** (0070 item 3, not done): a value stored to its slot on one
   path into a join is stored again after it.
3. **Splits at loop boundaries.** A value spilled in a loop is reloaded after every inner
   loop (the split points are the loop's edges), even where nothing between needs its
   register. Splitting at the uses instead of the loop boundary would keep these values
   in registers between them.
4. **Rematerialized constants per block.** A constant that is rematerialized rather than
   kept in a register is rebuilt in each block that uses it; the global reload removal of
   0070 (`drop_reloads`) does not apply to rematerialized values.
5. **Incoming stack arguments reloaded** at each use rather than once into a register
   (`hash` and `occ` of `generate_moves_sub`).
6. **One mask register** (0071 item 1, not done): every AVX-512 compare goes to `k1`, so
   masks used by turns are recomputed before each use. This needs a fifth register class
   (spilled to a GPR or memory).

Also seen: no loop rotation, so loads from parameters at the head of a loop are not
hoisted out of it; and a RIP-relative global address used as an indexed base is computed
(`lea r,[rip+...]`) inside the loop that uses it rather than once before it.

# 0077: Self-hosted register allocator: what is left after 0070 (movegen, mask registers, loop splits)

- Type: feature (performance)
- Status: fixed in part (635bcd1); the rest in 0088
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

## Resolution

635bcd1, measured on the benchmark's `bench_kernels` (deterministic counts by single-stepping)
against main before it (7eecc69):

- **Shrink-wrapping** (new, `mach.ax` `shrink_wrap`, x64.ax): the frame is set up only past
  the blocks that can return without it. `generate_moves_sub` returns at once in 2728 of its
  3813 calls (`rolls[depth] == 0`); those now run 6 instructions instead of 22. The frame
  tables mark each stretch run without the frame (`ftab.ax`), so traps there (bounds checks
  call `axiom.trap` without the frame) and profiles find the return address
  (`tests/traces/traces.ax`, "trap before the frame"). `save_moves` pays the copies into
  callee-saved registers without using its early return (its caller tests the same).
- **Global addresses not hoisted**: done (`opt.hoist_constants` with `globals`): a global
  indexed by a variable is addressed from a register built once before the loop
  (rematerialized if it loses its register); `ZOBRIST` in `generate_moves_into`.
- **Call arguments shuffled through temporaries** (item 1): a copy only copied again
  before its register dies is forwarded (`forward_copy`), and `drop_reloads` drops copies to
  a register that already holds the value.
- **Stack arguments / reloads read once** (part of item 5): folded into `test`, `cmp`
  and ALU operands after allocation (`fold_stack_reads`, physical-register liveness).
- **Stores repeated at joins** (item 2): no longer seen in `generate_moves_sub` after 0081.

| | before | after | LLVM |
|---|---|---|---|
| 100 `generate_moves_into` | 997k | 971k | 901k |
| `generate_moves_sub` | 583k | 548k | 546k |
| `generate_moves_into` alone | 84.5k | 79.9k | 32k (gathers) |
| `save_moves` | 220k | 233k | 226k |
| contact half-inputs (300) | 491k | 487k | 448k |

Timings (min of 9, 4 shared cores, about ±4%) moved within noise: movegen 851 -> 861 ns
(LLVM 929, Rust 835), 2ply 0.130 -> 0.126 s, gbr 0.98 -> 0.96 s, 1ply 0.032 -> 0.034 s. An
earlier measurement on the main of the day before gave movegen 895 -> 838 from
shrink-wrapping alone; on top of 0081-0085 it no longer shows. the benchmark results bit-identical.

Left (0088): mask registers, rematerialized values rebuilt per block (`af_input`, the
eighth argument of `calculate_half_inputs_with_masks`, is loaded 9 times per call), splits
at loop edges, loop rotation.

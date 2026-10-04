# 0066: Self-hosted register allocator: reloads around calls in movegen's recursive loop

- Type: feature (performance)
- Status: in progress
- Area: compiler (selfhost: mach, x64)

Follow-up to 0054. the benchmark's movegen kernel (`axiom/src/bin/bench_kernels.ax`) takes
about 884 ns per position with the self-hosted compiler against about 700 with the C
compiler (LLVM) and 690 in Rust. Most of the time is in `MoveGenerator.generate_moves_sub`
(`src/core/moves.ax`), a loop over the checkers that calls itself once per legal step and
then, usually, `save_moves`.

Deterministic counts (ptrace single-stepping 100 `generate_moves_into` calls):

| function | selfhost insts | stack loads | stack stores | LLVM insts | stack loads | stack stores |
|---|---|---|---|---|---|---|
| `generate_moves_sub` | 830k | 93k | 25.6k | 539k | 46k | 24k |

Per legal step (3596 recursive calls in the sample) the selfhost does about 25 stack loads
and 3 stores, LLVM about 13 loads and 1 store:

- **Reloads before the joins.** About 19 values are live across the recursive call, five of
  them in callee-saved registers. The rest are reloaded in a block right after the call
  (8 loads), though most are not used until after the `save_moves` call that usually
  follows; there they are reloaded again (6 loads). The joins after the
  `if ok and cond1 and cond2 { save_moves(...) }` have predecessors where the value
  is in a register (the condition blocks), so the reload is not moved down to the join.
- **Pieces reloaded per block.** Some in-loop pieces (`self`, `depth`, the loop's `dest`)
  lose a second time and are reloaded in each block that uses them, so `self` is loaded
  three times per step.
- **Incoming stack arguments spilled.** `hash` and `occ` arrive on the stack, are loaded at
  entry and stored again to new spill slots; LLVM reloads them from the argument slots.
- **Dead `mov`+`add` (lea) chains.** Address arithmetic folded into memory operands leaves
  `add`s whose results nothing reads (`lea r8,[rdi+0x180]`); `mc.dce` keeps them because
  `add` writes the flags, and their values take registers or even stack slots.

## Proposal

In `mach.ax`: place reloads where the value is needed (a block entered only from places
where the value is in memory reloads it itself), not at the end of the region after the
call; reload incoming stack arguments from their slots; drop flag-writing arithmetic whose
flags and results are dead; let the allocator prefer cheaper candidates to spill.

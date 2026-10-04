# 0066: Self-hosted register allocator: reloads around calls in movegen's recursive loop

- Type: feature (performance)
- Status: fixed (50cbebd)
- Area: compiler (selfhost: mach, x64, gvn)

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

## Fixed (50cbebd)

- **Greedy allocation** (`mach.ax` `greedy`, replacing the linear scan): intervals are
  taken largest first; each gets a free register (its hint's first) or evicts the
  intervals of lower spill weight that it overlaps anywhere (they are queued again),
  else it is spilled and split as before. Each register keeps its assigned segments
  in order, so interference is a binary search. The linear scan weighed an interval
  only against what was live where it started.
- **Coalescing** (`coalesce`) before allocation: the sides of a copy between virtual
  registers become one where their lives do not overlap, or where they hold the same
  value (the copy is the only definition of one, the other has one definition and is
  not redefined while the copy is live). Hottest copies first. Removes the phi copies
  of `used` in the loop latch, and many copies elsewhere.
- **Reloads past joins** (`split`): a block that neither uses the value nor calls,
  entered only from where the value is in memory, leaves it there, so the reload goes
  where it is used next. After the recursive call the loop's invariants are reloaded
  once at the join after `save_moves`, not on both paths.
- **Stack arguments** (`x64.ax`) are rematerialized from their incoming slots, not
  spilled to new ones. **Dead `add`/`sub`/`and`/`or`/`xor`** whose flags nothing
  reads are removed by `mc.dce`.
- **Sinking** (`gvn.ax` `sink`): an instruction no longer moves past a call into a
  block every path reaches anyway when that only makes its operands live across the
  call. `mask ^= 1 << i` had moved below the recursive call, keeping both `mask` and
  `1 << i` live across it.
- `AXIOM_DEBUG_RA=<function name suffix>` prints the allocator's decisions (x86-64).

Deterministic counts (100 `generate_moves_into` calls), main before this commit (with
0060) against after:

| | instructions | stack loads | stack stores |
|---|---|---|---|
| `generate_moves_sub` | 794k -> 721k (-9%) | 120k -> 83k | 46k -> 23k |
| movegen in all | 1257k -> 1180k | 132k -> 95k | 62k -> 37k |
| contact_inputs (300 calls) | 1165k -> 1160k | 5.5k -> 4.6k | 9.4k -> 8.8k |
| contact_net (16 batches) | 1074k -> 1081k | 5.0k -> 9.5k | 5.9k -> 8.6k |
| pruned_single (400 evals) | 303.5k -> 303.1k | 4.0k -> 3.6k | 6.0k -> 6.0k |

Timings (min of 11-15 on the loaded machine, noisy): movegen 733-818 -> 695 ns (LLVM
583-670 in the same runs); the other kernels within noise, contact_net perhaps 3%
slower: `dim128_pair` reloads two of its 16 activations per output again (coalescing
joins the accumulator chains of its loops into long intervals). the benchmark results
bit-identical. The self-hosted compiler builds itself in about 8 s instead of 12. The
compiler built for aarch64, run under qemu, now generates the same x86-64 compiler
bit for bit (before this change it did not: the linear scan's choices differed
between hosts) and reaches the same aarch64 fixed point.

Left: per legal step `generate_moves_sub` still has about 17 stack loads against
LLVM's ~12: the 12 or so loop invariants between calls do not fit in the caller-saved
registers, and the losers are reloaded in each block using them; a pressure-aware
region split would place them better. Most of the remaining instruction gap (721k
against LLVM's 539k) is not the allocator: `off`/`used`/`legal` materialised as 0/1
and tested again, and the shape of the illegal-step path.

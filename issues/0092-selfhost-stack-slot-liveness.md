# 0092: Self-hosted optimizer: stack-slot liveness (copies between slots, frames that share space)

- Type: feature (performance)
- Status: in progress
- Area: compiler (selfhost: copies, passes)

Every alloca is a stack slot of its own for the whole function, and the only way two
slots become one is 0074's `renamable` (a whole slot copied into a variable's slot that
was not used before, the source not used after, nor around a loop), which looks at
dominance and reachability, not at when a slot's contents are needed. So:

- **Copies between slots whose source is dead stay.** In the benchmark's
  `find_best_moves_0ply_cross_roll_scored` (`src/eval/prune.ax`, `cross_roll_evaluate`
  inlined), `board.swapped()` builds the swapped board in a `[2][25]u8` from two halves of
  the argument and copies it into the `Board` passed on: the board is copied twice. The
  copy out is split at the halves (0074's `split_copies`), and each half's source is only
  needed by the other half's copy, so neither `renamable` (which wants a whole-slot copy)
  nor `split_copies` (whose source is not known to be apart from the escaping destination)
  removes it.
- **Frames are the sum of their slots.** After inlining, a function holds the temporaries
  of everything inlined into it, most of them needed for a few instructions. bench_search's
  frames add up to 743 KB; `bench_search.run` has 145 KB, `find_best_move_ply_for_rollout`
  102 KB, `play_trial` 76 KB, `cross_scratch` 39 KB. Big frames touch more cache lines and
  pages (each frame over 4 KB is probed page by page on entry). LLVM's StackColoring
  lets slots with disjoint lifetimes share frame space (it has lifetime markers; we must
  compute lifetimes).

What is needed is liveness of stack slots over the CFG: a slot is live from where some
write of it may have run to where it may be read before being overwritten whole; a slot
whose address goes where we cannot follow is live from there on. Then (1) a copy of slot S
into slot D (at the same offset) where S and D are never needed at the same time makes
them one slot and goes, and (2) slots that never interfere share their bytes.

Two miscompiles to guard against (the previous attempt at merging slots met both):
- a slot renamed inside a loop, the next iteration reading a value modified meanwhile
  (the source is live around the loop's back edge);
- copies split in parts whose halves overlap once the slots are merged (a copy between
  different offsets of two slots that share bytes).

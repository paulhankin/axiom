# 0092: Self-hosted optimizer: stack-slot liveness (copies between slots, frames that share space)

- Type: feature (performance)
- Status: fixed (bd0e213)
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

Fix (`slots.ax`, bd0e213): run once the IR passes are done (they tell slots apart by their
allocas). Per slot over the CFG: live backwards from reads to writes that fill it whole,
only where some write of it may have run, and everywhere from where its address escapes.
A write of X where Y is live after it (or read by the same instruction) makes them
interfere, except a copy of Y into X at the same offset, which writes what is there once
they share. The graph is coalesced along such copies (hottest first, then the copy goes),
then coloured greedily, largest first. A module-wide analysis finds pointer parameters no
call can keep (only read, written and copied through, compared, passed on to parameters
that keep none); passing a slot's address there uses it at the call only. Tests:
`tests/run/slot_liveness.ax` (a copy in a loop whose source is read again after the copy
changed, halves swapped through a temporary, kept and returned addresses, address
comparisons); checked to fail with the liveness or escape parts of the interference
removed.

Effect (the benchmark bc2f24b, this machine's Xeon, AVX-512): the swapped board in
`find_best_moves_0ply_cross_roll_scored` is built once, in the slot passed on.
bench_search's frames 746 KB -> 530 KB (LLVM-backed Axiom: 397 KB);
`find_best_move_ply_for_rollout` 102 -> 43 KB, `bench_search.run` 145 -> 48 KB,
`play_trial` 76 -> 53 KB, `cross_scratch` 39 -> 28 KB; 543 allocas left of 1136. Results
bit-identical. Search time is within noise (user time, min of 7: 1ply 0.157 -> 0.146 s,
2ply 0.483 -> 0.485 s, gbr 2.13 -> 2.05 s; kernels unchanged). Matrix (one run, ratio to
Rust): 1ply 1.23x (LLVM 0.84x), 2ply 1.06x (1.04x), gbr 1.07x (1.15x).

Left: copies of a whole slot into a field of another (different offsets: 255 in
bench_search, e.g. `ScoredMove` built from a `Move`) would need slots placed at an offset
inside others; parameters passed to indirect calls and declarations still escape.

# 0074: Self-hosted optimizer: copies out of slots filled piece by piece, and whole-slot copies between locals

- Type: feature (performance)
- Status: fixed (7810756, 6e9b5ac)
- Area: compiler (selfhost: copies)

Outside the evaluation kernels, the benchmark's search code (`src/eval/search.ax`,
`cubeful_search.ax`, `prune.ax`, `rollout.ax`, `core/moves.ax`) builds small aggregates
(`Move`, `SubmoveList`, `ScoredMove`, `CubeInfo`, `EvaluationResult`, the `{ i16 }` and
`Vec` headers of `reserve`'s result) in stack slots and copies them on:

- **Copies out of a slot filled piece by piece.** A struct literal is stored one field at a
  time, or two halves are copied into it, and then the whole is copied (`memcpy`) or loaded
  as a vector somewhere else. The copy reads across several narrower stores, which the
  processor cannot forward to one wider load: each such copy waits for the stores to
  retire (a store-forwarding stall of a dozen cycles or more). LLVM's SROA and memcpy
  optimizations copy the fields themselves instead. `MoveGenerator.save_moves` has six such
  copies on its hot path (`store ... %355; memcpy %390 %355 10` for the `SubmoveList` of a
  move, then the 50-byte board).
- **A value returned into one local, then copied into the variable it is bound to.** A
  call's result in slot S copied whole into slot D, S never used again and D not used
  before: S can be D. `find_best_move_ply_for_rollout` had 230 copies and 91 slots,
  `InvariantCubeMet.new` 22 copies, `find_best_move_cube_using` 24.

The gbr evaluator (`bench_search --mode gbr`) was 1.20x Rust's time per position against
LLVM-backed Axiom's 1.05x, rollouts 1.31x against 1.29x.

Fix (`copies.ax`): `split_copies` splits a copy or vector load out of a slot filled piece by
piece at the pieces, each part taken from where it was written (a stored value, a fill, a
copy's source that has not changed since and cannot overlap the destination), skipping
bytes nothing writes (padding); `unread_writes` then removes writes to slots of bytes
nothing reads; `renamable` makes a whole slot copied into a variable's slot (not used
before the copy, the source not used after it, nor around a loop) that slot. On x86-64 a
small copy from or to a stack slot uses parts that do not overlap (16 + 8 + 1 bytes for 25,
not two overlapping 16-byte moves), so a copy out of what one copied in reads each part
from one store. `tests/run/slot_copies.ax` checks the cases that must not change: a copy
inside a loop whose source is needed again, halves swapped through a slot.

Effect (bench_search, min of 5, against main before them; this machine's Xeon, AVX-512):
1ply 1.23 -> 1.18 of Rust's time, 2ply 1.24 -> 1.22, rollouts 1.26 -> 1.21; then the
x86-64 part and the parts copied from anywhere: 1ply -4%, 2ply -1.5%, gbr -2%.

# 0074: Self-hosted optimizer: copies out of slots filled piece by piece, and whole-slot copies between locals

- Type: feature (performance)
- Status: open
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

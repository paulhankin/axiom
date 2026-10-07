# 0110: Self-hosted SROA: aggregates of more than 16 scalars stay in memory

- Type: feature (performance)
- Status: fixed (1fb5609)
- Area: compiler (selfhost: sroa, inline)

`sroa.ax` split a slot into scalars only up to `MAX_LEAVES = 16` leaves. the benchmark's
`hitting_shots_not_on_bar` keeps the six dice's direct shots in a
`DirectShots { k: [7]i32, pips: [7]i32, mult: [7]bool, blocked: [7]bool }` (28 leaves),
accessed only at constant offsets once its helpers are inlined: it stayed a stack slot,
written with a vector store, `memset`s and byte stores and read back by every roll. Its
cost (1287 instructions) also kept it above the inliner's limit for a function called once
(1200), so it was not inlined into `calculate_half_inputs_with_masks` (0086 item 1).

Fix: `MAX_LEAVES = 32`. The slot is scalarized, the function's cost drops to 1164 and it
is inlined.

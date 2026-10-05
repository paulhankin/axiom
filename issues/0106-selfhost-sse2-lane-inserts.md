# 0106: Self-hosted x86-64 without SSE4.1: integers put into vector lanes through the stack

- Type: feature (performance)
- Status: fixed (see below)
- Area: compiler (selfhost: x64)

Follow-up to 0086 (the benchmark's `axiom/` port built with `--cpu=x86-64`). Without
SSE4.1 there is no `pinsrd` / `pinsrq`, and `insert_lane` (x64.ax) put a 32- or 64-bit
integer into a vector by storing the vector to a stack slot, storing the lane over it and
loading the whole back. The load spans both stores, so it cannot be forwarded from the
store buffer and waits until both have been written (some 12 cycles). the benchmark's
`hitting_shot_masks` (six `an_board[i] == 2` bytes compared as one vector, read with one
8-byte load put into lane 0 of zeros) paid this on every call of
`calculate_half_inputs_with_masks`.

Fix: an integer into lane 0 of a zero vector is `movd` / `movq` alone (they zero the rest);
into another lane, `movd` / `movq` into an xmm register and then the shuffles a float lane
takes (`shufps` twice for 32 bits, `shufpd` / `punpcklqdq` for 64). Bytes still go through
memory (`pinsrw` would need the neighbouring byte).

Effect (`--cpu=x86-64`, together with 0108-0111): bench_search 0-ply 0.935 -> 0.89 of
Rust's time; tests/lowering/x64_baseline_sse2.ax checks that no such store-store-load
sequence is left, tests/run/sse2_lanes_selects.ax the results.

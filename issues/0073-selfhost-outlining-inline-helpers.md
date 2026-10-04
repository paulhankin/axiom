# 0073: Self-hosted outliner: inline helpers' hot bodies outlined as cold functions

- Type: bug (performance)
- Status: fixed (4b49fd9)
- Area: compiler (selfhost: outline)

`outline.ax` moves the region of a small function's uncommon path that makes calls into a
`.cold` function. It runs before inlining, and counted as calls both the trap calls of
bounds and conversion checks and the calls of `inline` functions not yet inlined. So the
whole `if x != 0 { ... top_bit(x) ... }` bodies of the benchmark's scalar hitting-shot helpers
(`indirect_double`, `prep_direct`, `bar1_direct` in `src/eval/features.ax`) became
`indirect_double.cold0` and the like: called on the hot path once inlined, with the
caller's accumulators passed by address (kept in memory). In the baseline x86-64 build
(which uses the scalar `hitting_shots_not_on_bar`; AVX-512 builds use the vector one)
these were about 7% of `contact_inputs`.

Fix: a call in a block that ends in `unreachable` (a trap) or to an always-inline function
does not count.

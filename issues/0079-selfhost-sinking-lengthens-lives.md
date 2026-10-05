# 0079: Self-hosted optimizer: sinking makes operands live instead of results

- Type: bug (performance)
- Status: fixed (d00826d)
- Area: compiler (selfhost: gvn)

`gvn.sink` moves a pure instruction down to the nearest block dominating its
uses. Where some path from its block skips that block, this saves work; where
every path reaches it anyway, it saves nothing, and the instruction's operands
(rather than its result) are then live on the way down. 0066 stopped such moves
past calls only.

In the benchmark's `hitting_shots_not_on_bar` (`axiom/src/eval/features.ax`) the
roll loop is unrolled into 21 straight-line diamonds, each adding to
`sums.np`, `sums.n1` and `sums.n2`. Each add's only use is the next add, so
sinking moved the last one to the end of the function, then the one before it
(its use had moved there), and so on: all 63 adds ended up in the last block,
with every roll's `n_chequers` and `n_pips` live to the end of the function.
The register allocator spilled them, and the rolls' working values with them
(the function's code was a third stack traffic).

Fix: where no path skips the target block, an instruction moves only if its
operands are live there regardless.

Effect (the benchmark, `--cpu=x86-64 -O2`, measured with 0080's first part):
bench_kernels `contact_inputs` 583 -> 575 ns; `pruned_contact` and
`pruned_race` (whose dot products had sunk the same way) about 10% faster.

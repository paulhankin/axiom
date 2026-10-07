# 0063: Self-hosted optimizer: correlated branches not threaded, data-dependent table branches not if-converted

- Type: feature (performance)
- Status: fixed (b18d0de, 360066b)
- Area: compiler (selfhost: thread)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).

## Today

**Tests of one value against several constants are each made.**
`calculate_race_inputs`, 23 points x 2 sides per position:

```
if nc == 1 { inputs[k] = 1.0; }
if nc == 2 { inputs[k + 1] = 1.0; }
if nc >= 3 { inputs[k + 2] = 1.0; }
if nc > 3 { inputs[k + 3] = @float_from_int(f32, nc - 3) / 2.0; }
```

selfhost: four compares and four branches on every point, `rex cmp bl,0x3` twice in a
row:

```
rex cmp bl,0x1 ; jne +  ; vmovss [r9],xmm3
rex cmp bl,0x2 ; jne +  ; vmovss [r8],xmm3
rex cmp bl,0x3 ; jb  +  ; vmovss [rdi],xmm3
rex cmp bl,0x3 ; jbe +  ; ...
```

LLVM threads the known outcomes: after `nc == 1` holds, it jumps past the other three
tests, and after `nc >= 3` fails it skips `nc > 3`. That gives one or two branches per
point, and the same compare's flags are reused (`cmp r11b,0x3; ... je`).

**A branch on table data in a hot loop.** `compute_board_hash` (movegen, per root
position and roll):

```
for pt in 0..25 { let c = b.raw[side][pt]; if (c > 0) and (c <= 15) { h ^= zob(side, pt, c); } }
```

The branch depends on the board, so it mispredicts. In the selfhost's
`generate_moves_into`, 46 of 140 PC samples are on the `test dil,dil` after the load.
`ZOBRIST` is a constant global, and its `c = 0` entries are zero, as the source
comment says. LLVM vectorizes the loop with masked `vpgatherdq`. That is out of scope
here, but if-conversion is not: the load index is bounded (`c <= 15`, or `c & 15` with
`c` a `u8` in a 16-entry stride), so `h ^= select(cond, ZOBRIST[i], 0)` is safe to
speculate.

## Proposal

`thread.ax`:
- Jump threading over correlated compares: when a block's condition is an `icmp` of `x`
  against a constant, and a successor path re-tests `x` against another constant (or
  the same one), and the first outcome decides the second, thread the edge past the
  second test. Facts from dominating conditions exist in GVN already (`gvn.ax` "facts
  from branch conditions"); the threading needs them per edge.
- If-convert a conditional `x ^= load` / `x += load` where the load is from a constant
  global and its address is in bounds on both paths (from the bounds facts, or because
  the index is masked to the array size): speculate the load and select.

## Impact

`calculate_race_inputs` (up to about 46 extra compares and branches per position, some
mispredicted) and movegen's hash (about 1.5% of movegen's samples are the mispredict).
Estimate: 3-6% of `race_inputs`, 1-2% of movegen, about 10-20 ns summed.

## Resolution

Fixed in b18d0de (with dd94248, 360066b), in `thread.ax`:

- **Correlated comparisons**: when the edge into a block that tests `x` against a
  constant comes from branches that tested `x` against constants (the predecessor's own,
  and those above it through single-predecessor blocks), and those outcomes decide the
  test (intervals of `x`, unsigned or signed), the edge is threaded. In
  `calculate_race_inputs` each of the four tests now jumps past the ones it decides.
- **Nested conditions**: `if (c > 0) and (c <= 15)`, two branches, becomes one branch on
  the and when both test one value against constants; `combine.ax` makes the and of two
  such comparisons one range check (`c - 1 < 15`), and `gvn.ax` takes bounds from range
  checks. (Other ands stay two branches: two predictable branches are cheaper than a
  branch on two flags; `classify_position` showed it.)
- **Table loads under data-dependent conditions**: if-conversion accepts an arm with one
  load from a constant global whose address is additions to it; the load's address
  becomes `select(cond, address, global)`, so it is always in bounds, and the arm becomes
  a select. `compute_board_hash` is now branch-free.

While at it, `outside_uses` looks for uses of a block's values only in the blocks it
dominates (and their successors' phis) instead of the whole function, halving jump
threading's time.

Measured: race_inputs see 0055 (the two together: -23%); movegen within noise (970 vs 971
ns, the hash's branch was a small part).

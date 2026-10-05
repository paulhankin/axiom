# 0075: Self-hosted optimizer: calls of a function itself in tail position stay calls

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: passes)

the benchmark's endgame-table index (`position_f` in `src/eval/endgame-table.ax`, GNU Board-game's
`PositionF`) recurses once per point:

```
if (f_bits & (1 << (n - 1))) != 0 {
    return combination(n - 1, r) + position_f(f_bits, n - 1, r - 1);
}
return position_f(f_bits, n - 1, r);
```

LLVM turns both calls into a loop (tail recursion elimination, with an accumulator for the
`+`) and inlines the loop into `position_endgame-table`. The self-hosted compiler keeps a call per
level, with a frame and the recursion's `combination` lookups in between, so the endgame-table
evaluations of a 2-ply search (`sanity_check_with_endgame-table`, `eval_race_bg_with_endgame-table`) took
about twice LLVM's time: 2.6% of the whole search more.

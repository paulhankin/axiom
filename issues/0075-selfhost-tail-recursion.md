# 0075: Self-hosted optimizer: calls of a function itself in tail position stay calls

- Type: feature (performance)
- Status: fixed (50ef28c)
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

Fix (`tailrec.ax`, early in `passes.optimize_func`): `return f(...)` and `return x op f(...)`
(op an integer +, *, &, | or ^, gathered in an accumulator that the other returns combine
with) become a jump back to the start with the arguments as parameters. Not for functions
with stack slots, calls passing arguments by address, or no other return (which can only
end by running out of stack, as `tests/traces/crash.ax` does). `position_f` is a loop
inlined into `position_endgame-table`, as with LLVM. `tests/run/tailrec.ax`.

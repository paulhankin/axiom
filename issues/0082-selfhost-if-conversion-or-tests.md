# 0082: Self-hosted if-conversion: stack-slot loads and `if (a or b)` arms

- Type: feature (performance)
- Status: fixed (3ca2d14)
- Area: compiler (selfhost: thread)

the benchmark's hitting shots (`nondouble_shots` in `axiom/src/eval/features.ax`) keep
their per-die results in a local struct and test them per roll:

    if k0 >= 0 { n_chequers = 1; hitter_used = k0; n_pips = ds.pips[d0]; }
    if (hitter_used != k1) or ds.blocked[d1] { n_chequers += 1; }

Both depend on the position and branch unpredictably; LLVM turns both into
cmovs. The self-hosted if-conversion kept both branches:

- arms could speculate only loads from constant globals, and `ds.pips[d0]`
  is a load from a stack slot;
- `merge_conds` merged two tests into one branch only when they become one
  range check (two branches predict better than one on an `and` of two
  flags); here the merged branch would be if-converted in turn;
- the diamond left by jump threading (`n + 1` on one arm, `select(blocked,
  n + 1, n)` on the other) counted `n + 1` twice and went over `MAX_SPEC`.

Fix: a load of a scalar at constant offsets inside a stack slot is
speculatable; `merge_conds` also merges tests whose merged branch `convert`
takes next (the arm both reach is a cheap block going to the join), letting
the second test load from a stack slot and renaming the join's phi inputs;
instructions both arms compute count once.

Effect (the benchmark, `--cpu=x86-64 -O2`): bench_kernels `contact_inputs`
565 -> 537 ns.

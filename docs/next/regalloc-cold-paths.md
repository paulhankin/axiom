## Compiler

- Native code: a loop that pushes onto a `Vec`, with the grow path inlined into it, keeps
  its state in registers. The static estimate of how often each block runs no longer
  makes a loop on a seldom-taken path (the grow path's doubling loop) look hotter than
  the path it branched off from, two vregs that are only ever copies of each other (a
  phi and its input around a loop) share a register, and on x86-64 the bounds check
  after `push`'s room test reuses that test's flags. A benchmark of that loop
  (`cmd/axiom/bench/vecpush.ax`) runs 14% fewer instructions; other benchmarks gain up to
  2.7% (one loses 0.4%).

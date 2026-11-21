## Compiler

- Builds at `-O2` and `-O3` are faster on several cores: the optimizer's first round
  over the functions, the rounds after outlining and constant arguments, and the
  re-optimization of each function after its calls are inlined run on threads (as
  many as for the parts of machine code, `AXIOM_JOBS`). Functions are now inlined
  level by level up the call graph, so that the functions of a level are
  re-optimized at once. On four cores (another build running) the compiler builds
  itself in about 25% less time (18.5 s to 13.7 s); the optimizer before the parts
  takes 6.9 s instead of 11.4 s. The executable does not depend on the number of
  threads. Builds with `-g` keep the optimizer on one thread.

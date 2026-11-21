## Compiler

- Builds at `-O2` and `-O3` are faster on several cores: the optimizer's first round
  over the functions, the round after constant arguments, and the re-optimization of
  each function after its calls are inlined run on threads (as many as for the parts
  of machine code, `AXIOM_JOBS`). Functions are now inlined level by level up the call
  graph, so that the functions of a level are re-optimized at once. The executable
  does not depend on the number of threads.

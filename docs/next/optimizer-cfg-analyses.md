## Compiler

- The optimizer does less work for the same output: the predecessors, block order and
  dominator tree of a function are kept until its control flow changes instead of
  being worked out again by each pass, jump threading searches only where it will
  thread something, and value numbering records what each comparison decides more
  cheaply. In an `-O2` build of the compiler the optimizer runs 15% fewer
  instructions (41.4 G to 35.0 G); the executable is byte for byte the same.
  `AXIOM_CHECK_CFG=1` checks every kept analysis against a fresh one (for debugging
  the compiler).

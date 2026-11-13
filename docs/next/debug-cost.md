## Compiler

- `-g` costs less: an executable's frame table keeps its lines for crash traces in
  compact read-only tables (the compiler's own executable with `-g` is 38 MB instead of
  109 MB, and its writable data is the same as without `-g`), and compiling with `-g`
  at `-O2` takes about a seventh more processor time than without (from a half) and a
  sixth more memory (from almost twice as much) for the compiler itself, a sixth more
  time and an eighth more memory for C programs: the analysis of where the variables
  of optimized code are does the work of what each instruction changes, not of every
  variable at every instruction. The debug information written is the same.

## Compiler

- `-g` costs less: an executable's frame table keeps its lines for crash traces in
  compact read-only tables (the compiler's own executable with `-g` is 38 MB instead of
  109 MB, and its writable data is the same as without `-g`), and compiling with `-g`
  at `-O2` takes about a third more processor time than without (from a half) and a
  fifth more memory (from almost twice as much) for the compiler itself, a quarter
  more time and a tenth more memory for C programs. The debug information written is
  the same.

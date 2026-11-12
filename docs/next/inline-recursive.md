## Fixes

- A small function with several calls to itself was inlined into its callers level after
  level, until the caller was tens of thousands of blocks: compiling such a program at
  -O2 could take gigabytes. Recursive functions are now inlined within bounds: a function
  in a cycle of calls that does not call itself is inlined into the others of the cycle,
  and a recursive one into each caller outside the cycle once, its calls to itself
  staying calls.

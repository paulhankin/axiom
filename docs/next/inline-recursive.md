## Fixes

- A small function with several calls to itself was inlined into its callers level after
  level, until the caller was tens of thousands of blocks: compiling such a program at
  -O2 could take gigabytes. A function that calls itself is no longer inlined.

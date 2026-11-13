## Compiler

- Jump threading at `-O2` goes through several blocks once a function is complete: a
  test that the edge into a block further up decides (the same test, or a null test of
  the same pointer, a comparison implied by an earlier one, a flag set to a constant on
  that path) is skipped on that path, the blocks in between (a diamond or two, up to 16
  instructions) copied for it. Repeated `p ? p->x : 0` tests, as in balanced trees'
  height checks, are made once on each path.

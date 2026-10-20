## Security fixes

- At `-O2`, the compiler took the popcount of a 64-bit value with no known bound to
  be at most 63, and dropped the bounds check of an index made from it.

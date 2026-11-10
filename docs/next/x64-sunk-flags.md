## Compiler

- Fixed: native x86-64 code (at `-O1` and above) could branch or select on the wrong
  flags. Where an operation was moved down to the test of its result, which it then
  replaced, a later test of an earlier operation's result was dropped as well, as
  though the flags were still that operation's. `(a or b) and (not c)`, with `c` an
  `or` of comparisons computed first, could be true when `a` and `b` were false.

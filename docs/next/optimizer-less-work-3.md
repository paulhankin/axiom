## Compiler

- Global value numbering does less work for the same output: what a branch's
  condition says of the comparisons of its two operands is one fact in its table, where
  it was a dozen made-up comparisons, each built as an instruction and entered under its
  own hash; and its walks skip the instructions that touch no memory sooner. In an
  `-O2` build of the compiler on one thread, value numbering runs 30% fewer
  instructions (14.8 G to 10.4 G), the optimizer 8% fewer (33.3 G to 30.6 G) and the
  whole build 4.5% fewer (100.7 G to 96.2 G); the executable is byte for byte the same.

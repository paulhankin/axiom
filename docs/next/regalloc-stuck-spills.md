## Compiler

- Native code: a loop through which more values are live than there are registers, all
  of them used in its body, no longer stops the compiler with "internal compiler error:
  register allocation did not converge" (seen with vector code for AArch64 at `-O1` and
  above, where loop-invariant vectors outnumber the 32 vector registers). Where one
  register too few was free at a point in the body, each round of the register
  allocator split one more of those values at the loop, which left it in a register
  through the body as before. Now, when the interval short of a register there evicts
  a value a second round in a row, that value is split where the split frees its
  register at that point, or else spilled around each of its uses, in the same round.
  The benchmarks compile to the same code; the compiler's own code is 0.03% smaller.

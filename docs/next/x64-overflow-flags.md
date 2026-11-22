## Compiler

- On x86-64, checked arithmetic tested where it is made branches on the flags of the
  operation itself: `@mul_checked`, `@add_checked` and `@sub_checked` (with `catch`,
  `!` or `--overflow=trap`), and C's `__builtin_mul_overflow`, `__builtin_add_overflow`,
  `__builtin_sub_overflow` and `ckd_mul`, `ckd_add`, `ckd_sub` whose operands and
  result have one type. A checked multiply is one multiply, which gives the product
  as well: `imul` then `jo` (signed, 16 to 64 bits), `mul` then `jb` (unsigned, 32 and
  64 bits), the one-operand `imul` or `mul` of `al` for 8 bits; before, the 64-bit one
  multiplied twice and tested a `seto` / `setb` of the overflow, and narrower ones
  made the product twice as wide besides the plain one. A checked add or sub is the
  `add` or `sub` and a `jo` or `jb`, not the arithmetic of the overflow test
  (`lea; xor; xor; test; jl` before). On a benchmark of checked arithmetic and error
  codes in hot loops (`cmd/axiom/bench/checked.ax`) this runs 17.9% fewer
  instructions.
- C's overflow builtins and `<stdckdint.h>` with operands and a result of one type of 8
  to 64 bits no longer compute the product in 128 bits, from four multiplies of 32-bit
  halves, on every target.

## Compiler

- Optimized code: a variable whose value is the product of two values that nothing
  holds any more is computed from them in the debugger (`DW_OP_mul`; a product of 64
  bits from its operands' 32-bit halves), and so is one computed through up to eight
  operations, or by an operation no instruction makes; a value sign-extended or
  shifted left is computed from the bits that matter, which the system's debugger
  gets right also where they are negative. Locations at `-O2`: 91.5% of the values
  stepped through on x86-64 are shown (C; 88.5% before, measured the same way) and
  90.5% on AArch64.

## Compiler

- On AArch64, checked arithmetic tested where it is made branches on the flags of the
  operation itself, as on x86-64: `@add_checked`, `@sub_checked` and `@mul_checked`
  (with `catch`, `!` or `--overflow=trap`) and C's overflow builtins and `ckd_*`.
  A checked add or sub is `adds` or `subs` and `b.vs` (signed) or `b.cs` / `b.cc`
  (unsigned), also with a constant operand; of 8 and 16 bits, on the operands shifted
  to the top of a register. Before, a signed one computed its overflow with
  `eor; eor; and; cmp; b.lt`, and an unsigned one compared before the add or sub. A
  checked multiply compares the high half and branches (`umulh; cmp; b.ne`, or
  `smulh; cmp ..., asr #63; b.ne` with the one `mul` that gives the product), where it
  made a 0 or 1 and tested it (`cset; cmp; b.ne`) and multiplied twice for signed 64
  bits; of 32 bits it is `umull` or `smull` and a compare, and of 8 and 16 bits one
  32-bit multiply of the extended operands that gives the product too. `@mul_sat`'s
  select reads the flags (`csel`). On a benchmark of checked arithmetic and error codes
  in hot loops (`cmd/axiom/bench/checked.ax`, reduced in size, under qemu) this runs
  17.5% fewer instructions.
- On x86-64, `@mul_sat` (a select on a multiply's overflow) is the multiply and a
  `cmovb` or `cmovo`, not `setb; movzx; test; cmovne`; and a checked unsigned add or
  sub of a constant, which the optimizer makes a compare with a bound, is the add and
  a `jb` or `jae`, not a `cmp`, a jump and the add after it.

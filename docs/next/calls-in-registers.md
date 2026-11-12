## Compiler

- Native code (x86-64, AArch64) returns small structures, slices and optionals of up to
  four scalars in registers, without going through memory, and a C function returning a
  structure in registers builds and receives it without a stack buffer.
- C: `a && f()` and `a || f()` used as values no longer branch on the last operand,
  and a function's constant or loaded narrow result is not extended twice.
- C: a structure assignment of more than 256 bytes is a forward copy (it can only
  overlap exactly), and `memmove` of a constant length up to 128 bytes is inlined on
  AArch64; a `memmove` onto itself no longer takes the slow backward copy on x86-64.

## C: inline assembly on WebAssembly

- asm statements, file-scope asm and assembly sources (`.s`, `.S`) work on wasm32 and
  wasm64, in the WebAssembly text format: plain and folded instructions with labels,
  every instruction of the core set and of the extensions `--features` enables (SIMD,
  relaxed SIMD, atomics, bulk memory, sign extension, saturating conversions,
  multivalue blocks, tail calls, `try_table`/`throw_ref`). An asm statement's operands
  are locals (`r`; `m` the object's address), constants (`i`, `n`) and symbols (`s`:
  `i32.const` of an address or a function's table index, `call` of a function);
  `asm goto` branches to its labels. File-scope asm and assembly sources are
  `(func ...)` and `(import ...)` fields; a naked function's asm is its code. Every
  template is validated (operand types, stack effect, blocks, labels, alignments):
  what is invalid is C3097, and the module is never invalid. See docs/c.md, "Inline
  assembly".

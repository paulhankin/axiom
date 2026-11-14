## Assembler

- asm statements, file-scope asm and assembly sources (`.s`, `.S`) work on wasm32 and
  wasm64, in the WebAssembly text format: plain and folded instructions with labels,
  every instruction of the core set and of the extensions `--features` enables (SIMD,
  relaxed SIMD, atomics, bulk memory, sign extension, saturating conversions, multivalue
  blocks, tail calls, exception handling in its standard form and, with
  `--features=+exception-handling`, its legacy form). An asm statement's operands are
  locals (`r`; `m` the object's address), constants (`i`, `n`) and symbols (`s`); `asm
  goto` branches to its labels. Every template is validated (operand types, stack effect,
  blocks, labels, alignments): what is invalid is C3097, and the module is never invalid
  (docs/c.md, "Inline assembly").
- File-scope asm and assembly sources take every module field the program's single
  memory and function table allow: functions and imports, globals (defined, imported from
  another unit or the host, exported), exception tags, passive data segments, passive and
  declarative element segments, exports of functions, globals and tags, a start function,
  and named function types; asm statements in C see the names file-scope asm declares.
  Active data and element segments are rejected by rule (the program owns the memory's
  and the table's layout).
- The WebAssembly linker merges globals and tags across units by name, checks each
  declaration's type against its definition, keeps passive segments apart from the
  memory's image, and emits global and tag imports and exports.

## Compiler

- `-g` now writes DWARF 5 debug information from the compiler's own code generators,
  for C and Axiom code: line tables (following C's `#line`), a subprogram per function
  and call frame information for every instruction on x86-64 and AArch64; with
  `--debug-info=full` (which `-g` is) also C's types, functions' parameters and local
  variables (with locations at `-O0`) and static objects. ELF executables and objects
  carry it in `.debug_*` sections, macOS programs in a `<out>.dSYM` bundle, WebAssembly
  modules in custom sections. `--debug-info=lines` writes the lines and frames only.
  The code generated is the same with and without it. See `docs/c.md`, "Debugging".
- C at `-O0` keeps every variable in its frame slot (where a debugger finds it).
- Windows PDB line tables follow C's `#line`.

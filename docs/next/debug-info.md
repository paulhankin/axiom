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
- With `-g`, C's block scopes are lexical blocks (a name declared again inside a block
  is told apart), typedefs are described and are the types of what is declared with
  them, block-scope statics, typedefs and tags are in their blocks, thread-local
  objects have locations (ELF), and a variable length array has its bound; a
  parameter passed by reference has a location at `-O0`.
- Optimized code (`-O1` and above) has locations for the variables assigned once (in
  the registers and spill slots that hold their values, as location lists, or as
  constants), for the parameters never assigned (their argument registers at the
  entry), and for the variables left in memory: never a wrong one.
- Axiom programs get their types, variables, globals and lexical blocks too (`-O0` now
  keeps an Axiom variable in its frame slot, as it does C's: about 8% more code at
  `-O0`, no measurable change in speed), and a language code of their own.
- Windows: the TLS directory no longer overwrites the first entries of the function
  table, which made the system's stack walks go round in the start-up's frames.
- WebAssembly: a module with a shared memory has a subprogram for its start function.

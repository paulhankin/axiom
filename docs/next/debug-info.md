## Compiler

- `-g` now writes DWARF 5 debug information from the compiler's own code generators,
  for C and Axiom code: line tables (following C's `#line`), a subprogram per function
  and call frame information for every instruction on x86-64 and AArch64; with
  `--debug-info=full` (which `-g` is) also C's types, functions' parameters and local
  variables (with locations at `-O0`) and static objects. ELF executables and objects
  carry it in `.debug_*` sections, macOS programs in a `<out>.dSYM` bundle, WebAssembly
  modules in custom sections. `--debug-info=lines` writes the lines and frames only.
  The instructions generated are the same with and without it (an object's code and
  relocations are identical; in an executable the line rows crash traces get move the
  data after them, and the addresses instructions hold with it). See `docs/c.md`,
  "Debugging".
- C at `-O0` keeps every variable in its frame slot (where a debugger finds it).
- Windows PDB line tables follow C's `#line`.
- With `-g`, C's block scopes are lexical blocks (a name declared again inside a block
  is told apart), typedefs are described and are the types of what is declared with
  them, block-scope statics, typedefs and tags are in their blocks, thread-local
  objects have locations (ELF), and a variable length array has its bound; a
  parameter passed by reference has a location at `-O0`.
- Optimized code (`-O1` and above) has locations for the variables assigned any number
  of times: each assignment is followed through the optimizer to the machine code, and
  from there on the variable is where its value is (registers, spill slots, a constant,
  a frame object's or a datum's address, a value plus a constant, the low bits of a
  widened loop counter, the merge of the values reaching a join), structures taken
  apart in pieces, variable length arrays through their addresses; never a wrong one.
  On the suite's C programs at `-O2` about nine variables in ten have a location
  somewhere, up from about half.
- Axiom programs get their types, variables, globals and lexical blocks too (`-O0` now
  keeps an Axiom variable in its frame slot, as it does C's: about 8% more code at
  `-O0`, no measurable change in speed), and a language code of their own.
- Windows: the TLS directory no longer overwrites the first entries of the function
  table, which made the system's stack walks go round in the start-up's frames.
- WebAssembly: a module with a shared memory has a subprogram for its start function.
- The line tables give each file's MD5 checksum (DWARF 5's `DW_LNCT_MD5`), and each
  unit has a name index (`.debug_names`) of its subprograms, static variables, named
  types and enumerators, for debuggers' lookups by name.
- A structure, union or enumeration defined in the members of one declared in a block
  is in that block's scope too.

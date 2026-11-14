## Debug information

- `-g` writes DWARF 5 debug information from the compiler's own code generators, for C
  and Axiom code: line tables (following C's `#line`, each file with its MD5), a
  subprogram per function and call frame information for every instruction on x86-64 and
  AArch64; with `--debug-info=full` (which `-g` is) also the types (typedefs, bit-fields,
  enumerators of any value), functions' parameters and local variables in their lexical
  blocks, static and thread-local objects, a name index (`.debug_names`), and C's macros
  (`.debug_macro`; `--debug-macros=off` leaves them out). ELF executables and objects
  carry it in `.debug_*` sections, macOS programs in a `<out>.dSYM` bundle (the DWARF of
  each object linked carried in at its place), WebAssembly modules in custom sections
  (each object's own, which the linker merges). `--debug-info=lines` writes the lines
  and frames only. The instructions generated are the same with and without it (an
  object's code and relocations are identical; in an executable the line rows crash
  traces get move the data after them, and the addresses instructions hold with it), and
  two builds are the same (`__DATE__` and `__TIME__` change nothing in it). See
  `docs/c.md`, "Debugging".
- `-O0` keeps every variable, C's and Axiom's, in its frame slot (Axiom: about 8% more
  code at `-O0`, no measurable change in speed).
- Optimized code (`-O1` and above) has locations for the variables assigned any number
  of times, never a wrong one: each assignment is followed through the optimizer to the
  machine code, and from there on the variable is where its value is (registers, spill
  slots, a constant, an address, the low bits of a widened loop counter, the merge of the
  values reaching a join, pieces of a structure taken apart, vector registers); a value
  nothing holds any more is computed from the values it was made from (DWARF
  expressions of additions, multiplications, bitwise operations, shifts, extensions; on
  x86-64 Linux also floating-point arithmetic and conversions, with DWARF 5's typed
  operations), and a parameter is its entry value once nothing holds it. Every call is a
  call site (`DW_TAG_call_site`). Stepping randomly generated programs at `-O2`, the
  system's debuggers show a value at 92.9% (x86-64) and 91.0% (AArch64) of the
  instructions where a C variable has been assigned (92.0% and 92.2% for Axiom).
  WebAssembly: optimized code's variables are in the function's locals
  (`DW_OP_WASM_location`).
- Functions the optimizer inlined are described: each copy is an inline instance
  (`DW_TAG_inlined_subroutine`) of an abstract instance of the function, so debuggers
  show the inlined frames in a backtrace and `step` into them; the PDB has them as inline
  sites. Crash traces mark the frames of functions inlined ("at f (file:line) (inlined
  into g)").
- An Axiom program's compilation units are marked as C11 (`DW_AT_language` 0x1d): with
  C's code both of the system debuggers tried show Axiom's variables, structures,
  slices, enumerations and error unions and evaluate arithmetic on them (one shows a
  tagged union's tag but not its payload). The producer names Axiom.
  `--debug-language=axiom` gives the units Axiom's own code (0x8a78).
- Windows (x64 and ARM64): with `-g` the PDB has the program's types and variables for C
  and Axiom code, in CodeView form (structures, unions, enumerations, bit-fields, arrays,
  pointers, function types, typedefs, static and thread-local objects, each function's
  type, frame and lexical blocks, its parameters and locals at `-O0`, and at `-O1` and
  above the variables in a register, a frame slot or memory through an address in a
  register: what CodeView cannot say is "optimized out"). PDB line tables follow C's
  `#line`, and a function's code of several files (inlined) is read in full by the
  system's symbol handler.
- Cost: `-g` at `-O2` takes about a sixth more processor time and an eighth more memory
  for C programs, a seventh more time and a sixth more memory for the compiler itself;
  an executable's frame table keeps its lines for crash traces in compact read-only
  tables (the compiler's own executable with `-g` is 38 MB instead of 109 MB, and its
  writable data is the same as without `-g`).
- macOS: thread-local variables have locations, through descriptors of the form the
  platform's debugger reads (not yet tried in it: nothing here runs macOS).
- Fixed: the name index hashed names without case folding (lookups of names with
  capital letters failed in debuggers that check the hash); two cold parts of functions
  could get the same name, and the PDB gave one the other's procedure; on Windows a
  function in a COMDAT group of its own had another function's unwind length, a
  function's cold part took its function's line table, and the TLS directory overwrote
  the first entries of the function table (`.pdata`), which made the system's stack
  walks go round in the start-up's frames; WebAssembly: a module with a shared memory
  has a subprogram for its start function, and after a C `longjmp` a stack trace no
  longer shows the frames the jump left.
- Windows: the thread-local template starts the `.data` section; on ARM64 prologues and
  epilogues record their call frame states (for the PDB's locations).

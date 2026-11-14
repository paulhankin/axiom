## Compiler

- With `-g`, an Axiom program's compilation units are marked as C11 (`DW_AT_language`
  0x1d) instead of with a code of DWARF's user range: a debugger that knows no such
  code showed none of their variables, and with C's code both of the system debuggers
  tried show Axiom's variables, structures, slices, enumerations and error unions and
  evaluate arithmetic on them (one of them shows a tagged union's tag but not its
  payload). The producer still names Axiom. `--debug-language=axiom` gives the units
  Axiom's own code (0x8a78) as before.

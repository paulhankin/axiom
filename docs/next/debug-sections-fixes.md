## Compiler

- Two cold parts of functions (`-O2`'s outlining) could get the same name, and the
  PDB then gave one of them the other's procedure; their names are now unique.
- Windows: a function in a COMDAT group of its own (`--function-sections`) has
  unwind information of its own length (it could have another function's), and an
  inline site whose code starts without a line of its own has its code in the PDB.

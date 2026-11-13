## Compiler

- Windows (x64 and ARM64): with `-g` the PDB has the program's types and variables for
  C and Axiom code, in CodeView form: structures, unions, enumerations, bit-fields,
  arrays, pointers, function types, typedefs (`S_UDT`), static and thread-local
  objects, each function's type, frame and lexical blocks, its parameters and locals at
  `-O0`, and at `-O1` and above the variables the optimizer's tracking places in a
  register, a frame slot or memory through an address in a register (never a wrong
  value: what CodeView cannot say is "optimized out"). The types of all objects are
  merged once (TPI and IPI with their hash streams), and the PDB has a module per
  object. See `docs/c.md`, "Debugging".
- Windows: a line table whose function holds code of several files (inlined) is read
  in full by the system's symbol handler (a subsection per file's run of lines), and a
  function's cold part no longer takes its function's line table.
- Windows: the thread-local template starts the `.data` section.
- Windows on ARM64: prologues and epilogues record their call frame states (for the
  PDB's locations).

## C: assembly directives and line information

- A difference of two labels that are defined further on in the same section
  (`.long end - start` before `start:`) now assembles, in file-scope asm and assembly
  sources, on every object format.
- A global symbol that a C unit's file-scope asm defines keeps the C library's
  definition of that name out, as an assembly source's does (on WebAssembly that was an
  error); an asm statement sees only its own unit's file-scope asm names.
- `.file` and `.loc` give the line information under `-g` on every object format,
  in the PDB's line tables on Windows too.

## C

- C and Axiom build into one program on every target, with the compiler's own C front
  end and linkers: `axiom build main.ax util.c` (an Axiom program with C translation
  units), `axiom build main.c rt.ax` (a C program with Axiom modules), or a package
  whose `axiom.toml` names a directory of C sources (`c-sources = "csrc"`). Axiom calls
  C through `extern "C"` and C calls Axiom's `export fn`s, with the C calling
  convention; a name defined on both sides is `C3065`, a C `main` beside an Axiom one
  `C3066` (docs/c.md, "C and Axiom in one program").
- An executable leaves out what its C units define and nothing reaches, at every
  optimization level, so an unused library costs nothing.
- When a C program defines `exit` (a unit or an Axiom module), returning from `main`
  calls it with `main`'s value. The order of start-up and exit is documented.

- Windows: an object that C declares and no unit defines is imported from its DLL
  (`extern int _fmode;`), and its address in a static initializer is `C3067`; the
  unwind information of a function with a variable length array no longer reads the
  saved registers relative to the moving stack pointer.

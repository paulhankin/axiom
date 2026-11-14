## Compiler

- Programs built with separately compiled modules (`--units=module`, the default at
  `-O0` and `-O1`, and `axiom test`) leave out the code and data of their modules that
  nothing uses, and the copies of generic functions that another module's copy won
  over: each function and datum of a module's object is in a section of its own, and
  the link collects garbage (`--gc-sections`, which `--no-gc-sections` turns off).
  Executables at `-O0` are often half the size they were (more for programs that use
  a little of large `std/` modules). Frame tables, call-site records and `.eh_frame`
  entries go with the code they describe. Whole-program builds (`-O2`) are unchanged.
- `--gc-sections` now leaves out the frame tables of the code it leaves out (they kept
  all code alive before), keeps one copy of a weak definition rather than every
  object's, and drops the `.eh_frame` entries of code it leaves out.

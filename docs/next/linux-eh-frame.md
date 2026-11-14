## Linking

- On Linux every function the compiler makes has its call frame information in
  `.eh_frame`, with or without `-g` (before, only `-g`'s `.debug_frame` had it), so
  the platform's unwinder walks through Axiom and C frames: backtraces, and exceptions
  of another language's code that pass through them. The procedures of assembly with
  `.cfi_` directives are in it too. The linker places the objects' `.eh_frame`
  together, ends it, makes `.eh_frame_hdr` and `PT_GNU_EH_FRAME`, and names both in the
  executable's section headers; its stub functions have FDEs too. Executables grow by
  about 9% for a program that prints a line (its runtime's small functions), 2% for a
  large one. `--unwind-tables=off` leaves the tables out (the program's own stack traces
  are the same; the system's unwinder then stops at Axiom and C frames). The call frame
  programs are shorter in `.debug_frame` and macOS's `__eh_frame` too.
- Exceptions of another language's code pass through C code on Linux: C++ objects of
  the system's compiler throw through this compiler's C functions (with variable length
  arrays, at every optimization level, x86-64 and AArch64) and catch above them. The
  objects' language-specific data and personality routines stay with their functions
  under `--gc-sections`, and their run time comes from the language's shared library.
  Such a program takes `--c-lib=system`: linking a shared library built against the
  system's C library with the compiler's (`--c-lib=axiom`) is an error that says so.
- With the compiler's C library, the platform's unwinder (its compiler's static run-time
  archive) links: its weak references to the system library's threads stay null, and the
  C library has `dl_iterate_phdr` and `_dl_find_object`, through which it finds the
  call frame tables of a static or dynamic program. The unwinder walks out of a signal
  handler through the C library's signal frame.
- `--c-lib=system` keeps what the objects linked with the program refer to among its own
  C units' functions (such a function that the program's own code did not call was left
  out, an undefined symbol).

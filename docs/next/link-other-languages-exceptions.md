## Linking

- Exceptions of another language's code pass through C code on Linux: C++ objects of
  the system's compiler throw through this compiler's C functions (with variable length
  arrays, at every optimization level, x86-64 and AArch64) and catch above them. The
  objects' language-specific data and personality routines stay with their functions
  under `--gc-sections` (they were left out before, and the exception then ended the
  program), and their run time comes from the language's shared library. Such a program
  takes `--c-lib=system`: linking a shared library built against the system's C library
  with the compiler's (`--c-lib=axiom`) is now an error that says so, where before the
  two libraries' streams and multibyte states could meet.
- With the compiler's C library, the platform's unwinder (its compiler's static run-time
  archive) links: its weak references to the system library's threads stay null, and the
  C library has `dl_iterate_phdr` and `_dl_find_object`, through which it finds the
  call frame tables of a static or dynamic program. The unwinder walks out of a signal
  handler through the C library's signal frame.
- `--c-lib=system` keeps what the objects linked with the program refer to among its own
  C units' functions (such a function that the program's own code did not call was left
  out, an undefined symbol).
- macOS: the platform's objects' personality routines and LSDAs are in the executable's
  unwind table (its personalities and LSDA index), and an FDE's pointer to its routine's
  GOT slot (`ARM64_RELOC_POINTER_TO_GOT`) is relocated right.

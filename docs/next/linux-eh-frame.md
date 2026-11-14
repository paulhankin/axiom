## Native code

- On Linux every function the compiler makes has its call frame information in
  `.eh_frame`, with or without `-g` (before, only `-g`'s `.debug_frame` had it), so
  the platform's unwinder walks through Axiom and C frames: backtraces, and exceptions
  of another language's code that pass through them. The procedures of assembly with
  `.cfi_` directives are in it too. The linker places the objects' `.eh_frame`
  together, ends it, and names it and `.eh_frame_hdr` in the executable's section
  headers; its stub functions have FDEs too. Executables grow by about 9% for a
  program that prints a line (its runtime's small functions), 2% for a large one.
  The call frame programs are shorter in `.debug_frame` and macOS's `__eh_frame` too.

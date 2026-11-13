## Windows

- The destructors of this compiler's COFF objects run in the same order (the reverse
  of their priorities, the plain ones first) whether the compiler's linker or the
  system's links them: their `.CRT$XT*` sections are named for the C runtime's forward
  order, and the compiler's start-up runs the termination table forward too.

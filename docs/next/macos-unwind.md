## Compiler

- macOS executables have unwind tables: `__unwind_info` with a compact encoding for
  every function, and `__eh_frame` entries for the frames a compact encoding cannot
  describe (a stack pointer that moves, a sibling call before a function's end,
  shrink-wrapped code, vector registers saved whole, assembly procedures with `.cfi_`
  directives). Mach-O objects carry `__compact_unwind` and `__eh_frame`, and those of
  the platform's compilers' objects are linked in, with their personality routines and
  LSDAs. On arm64 a function saves its callee-saved registers in pairs, as the compact
  encoding has them. The tables are decoded and checked by the suite; no unwinder has
  run over them yet (nothing here runs macOS).

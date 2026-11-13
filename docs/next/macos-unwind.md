## Compiler

- macOS executables have unwind tables: `__unwind_info` with a compact encoding for
  every function, and `__eh_frame` entries for the frames a compact encoding cannot
  describe (a stack pointer that moves, a sibling call before a function's end,
  shrink-wrapped code, vector registers saved whole, assembly procedures with `.cfi_`
  directives). Mach-O objects carry `__compact_unwind` and `__eh_frame`, and those of
  the platform's compilers' objects are linked in. On arm64 a function saves its
  callee-saved registers in pairs, as the compact encoding has them.
- The `.dSYM` of a program linked from several Mach-O objects with debug information
  is right for each of them (the objects' offsets between debug sections are moved to
  where their sections land), the platform compilers' objects included.
- macOS: thread-local variables have locations with `-g`, through descriptors of the
  form the platform's debugger reads.
- The name index (`.debug_names`) hashes names case-folded, as DWARF 5 specifies
  (lookups of names with capital letters failed in debuggers that check the hash).

## C library

- Windows on ARM64: `longjmp` has the system unwind the frames in between (as on x64),
  and `SIGSEGV`, `SIGFPE` and `SIGILL` handlers get the processor's faults in the
  program's code. Built and checked, not run (nothing here runs Windows on ARM64).

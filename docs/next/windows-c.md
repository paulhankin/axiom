## Windows

- The x64 unwind information is right at every instruction: a large frame's stack probe
  no longer moves the stack pointer before its allocation code, and sibling-call
  epilogues unwind right under any implementation of the system's unwinder (an entry of
  their own for each jump).
- Nothing comes from the system's C library (`msvcrt.dll`) unless `--c-lib=system` makes
  it the program's library: a name C code declares and nothing defines is an undefined
  symbol.
- `SIGSEGV`, `SIGFPE` and `SIGILL` handlers get the processor's faults in the program's
  code, and `longjmp` has the system unwind the frames in between (`RtlUnwindEx`), on x64
  and ARM64. The ARM64 forms are built and checked, never run (nothing here runs Windows
  on ARM64).
- Local time is the system's time zone setting when `TZ` is not set.
- `--stack-size=` sets the main thread's stack (Windows, macOS, WebAssembly).
- Crash traces show the faulting instruction's line when the program has line tables
  (`-g`), on every native target.

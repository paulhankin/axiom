# Windows: unwinding, signals, local time, longjmp

- The x64 unwind information is right at every instruction: a large frame's
  stack probe no longer moves the stack pointer before its allocation code,
  and sibling-call epilogues unwind right under any implementation of the
  system's unwinder (an entry of their own for each jump).
- Nothing comes from the system's C library (`msvcrt.dll`) any more: a name C
  code declares and nothing defines is an undefined symbol.
- `SIGSEGV`, `SIGFPE` and `SIGILL` handlers get the processor's faults (x64).
- `longjmp` unwinds through the system's frames (`RtlUnwindEx`, x64).
- Local time is the system's time zone setting when `TZ` is not set.
- `--stack-size=` sets the main thread's stack (Windows, macOS, WebAssembly).
- Crash traces show the faulting instruction's line when the program has line
  tables (`-g`), on every native target.

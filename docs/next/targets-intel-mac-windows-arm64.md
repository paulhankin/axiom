## Targets

- Two new targets: `--target=x86_64-macos` (Intel Macs, macOS 11 or later) and
  `--target=aarch64-windows` (Windows on ARM64), for Axiom and C. They build every
  program of the test suite, and the suite checks their executables' structure (the
  Mach-O load commands, binds and calls; the PE headers, imports, and on ARM64 every
  function's unwind information against its code), but **neither has been run on a
  real system yet**. On Windows on ARM64 a variadic function takes all its arguments in
  `x0`-`x7` and then the stack, floating-point ones as their bits, as the platform's
  C ABI says.
- `axiom test --no-run=<file>` builds the test binary without running it, for a target
  this machine cannot run.

## Fixes

- Windows: the TLS directory was written over the first entries of the function table
  (`.pdata`), and a function defined by more than one object (a generic instance) had
  two entries for the copy that was kept and none for the other.

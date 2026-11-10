## Compiler

- Optimized builds use less memory at their peak (14% less: 417 to 357 MB for the
  compiler building itself for x86-64 at -O2): a function inlined into all of its
  callers gives its code back at once, for the inlining still to come to reuse, where
  it used to stay until the end of the build.
- A WebAssembly program linked by the compiler's own linker is no longer written as an
  object file and read back: the linker takes it from memory (20% less memory at the
  peak with the change above: 494 to 393 MB for the compiler building itself at -O2).

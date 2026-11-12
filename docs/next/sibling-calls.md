## Compiler

- At `-O2` and above, native code (x86-64 and AArch64, Linux, macOS and Windows) turns
  a call in tail position into a jump once the caller's frame is restored: `return
  g(a, b);` to another function, a call through a pointer, C's `return x && f(y)`, and a
  structure returned in registers that the caller returns as it gets it.
  Mutual recursion and interpreters whose handlers call the next one run in constant
  stack, where they could run out of it before. Calls are kept where the callee could
  see the caller's frame (the address of a local, a variable-length array), where it
  needs more stack for its arguments than the caller was given, and in Axiom code with
  stack traces on, whose traces keep every call (spec §16.4.1); C traces at `-O2` can
  lack such a caller (`docs/c.md`). WebAssembly keeps ordinary calls.

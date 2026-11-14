## WebAssembly

- The constructors of linked WebAssembly objects (the linking section's `INIT_FUNCS`,
  this compiler's and other compilers') run before `main`, by priority, through the
  linker's `__wasm_call_ctors`; this compiler's objects list their C units' constructors
  and destructors so, which the system's WebAssembly linker honours too.
- A weak reference to a C function that nothing linked defines is not an import of the
  host's (`env`): it is null, and a call of it traps in a stub the linker makes, in the
  compiler's linker and, for its objects, in the system's WebAssembly linker. Weak data
  nothing defines is at address 0.

## WebAssembly

- A weak reference to a C function that nothing linked defines is no longer an import of
  the host's (`env`): it is null, and a call of it traps in a stub the linker makes, in
  the compiler's linker and, for its objects, in `wasm-ld`. Weak data nothing defines is at
  address 0.

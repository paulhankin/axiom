## WebAssembly

- The constructors of linked WebAssembly objects (the linking section's `INIT_FUNCS`,
  this compiler's and other compilers') run before `main`, by priority, through the
  linker's `__wasm_call_ctors`; this compiler's objects list their C units'
  constructors and destructors so, which `wasm-ld` honours too.

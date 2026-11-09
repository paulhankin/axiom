## Compiler

- Builds for WebAssembly are faster: reading an object back for linking looked up the
  function or data segment of each relocation with a linear search, which was 40% of an
  unoptimized build of a large program. The compiler building itself for wasm32 takes 30%
  less CPU time at -O0 and 17% less at -O2; a mid-size program at -O0, 14% less.
- Builds use less memory at their peak (7% less at -O2: 450 to 417 MB for the compiler
  building itself): the compiler now frees each function's machine-code lists, the
  encoder's buffers, its scratch memory and the object files it has written or read.
- `AXIOM_TIMES=1` also prints the memory in use and taken from the system after each
  phase, and how much of each of the compiler's arenas is used.

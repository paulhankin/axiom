## Compiler

- Very large functions compile much faster at -O2, mostly noticed on WebAssembly, where
  more is inlined: a function made of thousands of branches one after another took time
  quadratic in its length in three places (looking for cold code to move out of it,
  finding where a value's uses meet, giving values WebAssembly locals). A generated test
  of 30 checks that took 22 s for wasm32 takes 0.8 s.
- The WebAssembly backend no longer runs out of stack on such a function, or on an
  expression thousands of operations deep ("trap T0008 (stack exhausted)" from the
  compiler itself).

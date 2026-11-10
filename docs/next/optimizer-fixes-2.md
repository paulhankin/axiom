## Fixes

- Optimized code (-O1 and up) could compute `7 - y` as `y ^ 7` (and likewise for other
  constants of the form 2^n - 1) for a y greater than the constant: the proof of y's bound
  asked for it twice, and where its budget ran out in between, "not known" passed for a
  small bound. It needed a value reached through several branches and selects.
- -O2 could unroll an inner loop to the wrong number of iterations when its exit depended
  on a value of the enclosing loop that is the inner counter as it was when the inner loop
  last ended: the trip count was that of the first run.
- A `bool` returned by a C function, or by any function called through a pointer, is
  now reduced to its low byte, the only part the C calling conventions define: `if c_fn() {
  ... }` could take the wrong branch when the C compiler left other bits set in the
  register (x86-64 and AArch64).
- x86-64 with AVX-512: `(x & m) | (y & ~m)` on vectors, where another expression also used
  `x & m` or `y & ~m`, could read a register that was never loaded. -O2 makes such code
  from neighbouring conditional stores.
- Stack traces on native targets keep the frames of a recursion whose calls are in tail
  position (`return f(n - 1) + 1`): at -O1 and up the recursion became a loop, and a trap
  inside it showed one frame where -O0 showed them all. Such calls are now left as calls
  while stack traces are on, as they already were for WebAssembly; `--stack-traces=off`
  still makes the loop. The bootstrap compiler marks them so that LLVM leaves them too.

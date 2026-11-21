## Compiler

- Register allocation of values live through loops around calls: a value that a loop
  does not use, live across a call in it, stays on the stack all through the loop
  (stored before it if it must be, reloaded where the loop is left) instead of being
  reloaded after the call each time around, and a reload moves past a branch to the
  path that reads it. In std's `Map` lookups, the probe loop's call of the key
  comparison no longer has the probe's step, mask and group reloaded and stored around
  it: 0.4% fewer instructions on a map benchmark, 0.7% when the compiler compiles.
- x86-64: a function set up to return early without a frame (shrink-wrapping) whose
  frameless part runs out of registers now sets its frame up at the entry and uses
  the callee-saved registers there: one of the runtime's leaf loops went from 32
  stack accesses to 1.

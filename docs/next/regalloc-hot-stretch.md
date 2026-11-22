## Compiler

- Native code: in a loop whose values outnumber the callee-saved registers and live
  across calls on a seldom-taken path through it (a `Vec.push` loop with the grow path
  inlined), the register allocator keeps those values in registers through the loop's
  hot stretch and in memory on the cold path, reloading them where the cold path rejoins,
  instead of reloading them each time around. It does so only where the reloads and
  stores saved outweigh those added at the stretch's edges and a register is free for
  the value all through the stretch. The `Vec.push` benchmark
  (`cmd/axiom/bench/vecpush.ax`) runs 11% fewer instructions; the other benchmarks
  compile to the same code.

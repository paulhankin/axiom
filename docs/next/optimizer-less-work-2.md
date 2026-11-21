## Compiler

- The optimizer does less work again for the same output: constant folding goes
  straight to the tests for the operations most instructions are (loads, address
  arithmetic, phis), the walks over a function's blocks and edges make fewer calls, a
  kept block order says when it has every block, and the passes keep their scratch
  lists between calls instead of growing new ones. In an `-O2` build of the compiler
  on one thread the optimizer runs 7.5% fewer instructions (35.8 G to 33.1 G), and the
  whole build 3.6% fewer (103.4 G to 99.7 G); the executable is byte for byte the same.

## Compiler

- Fixed: programs that the optimizer compiled wrongly, found by a review of its passes.
  At `-O1` and above:
  - a loop keeping the previous iteration's value in a tagged union
    (`prev = U.Some(cur)`) read the current one;
  - a chain of tests (`x == 1 or x == 5 or (x -% 10) < 3`) folded into one could lose a
    value used after it (or crash the compiler at `-O2`);
  - a copy of a local whose address a callee kept shared its bytes, so writes through
    the kept address changed the copy;
  - `p.* = q.*` (and a copy through a temporary) where `p` and `q` overlap or are equal,
    and a store before an overlapping `@mem_copy` that reads it, lost data;
  - comparisons of a value carried around a loop used facts about the next one;
  - a load in a loop was moved before a call that may exit (the program crashed instead);
  - two equal pointers into different objects (one past the end of an array and the
    next object) were taken for one, and a pointer made from an integer computed from
    `@int_from_ptr(&a)` for one into `a`;
  - an overflow check of a wrapping multiplication by a large constant was wrong;
  - a sum of bytes that adds one twice, or whose bytes are loaded before a branch, came
    out wrong.
- Fixed in x86-64 code at every level: a load moved above a float or vector store to the
  same memory; an AVX-512 masked load above the setting of its mask; a value set before
  a loop that runs no times lost on a path that ends in a trap; a comparison at the start
  of a function's frame setup lost; a shuffle of a 512-bit vector to a 128-bit one on
  processors with AVX2 or AVX-512.
- Fixed: a call through a function pointer cast to another function type crashed the
  compiler at `-O2`, and on WebAssembly stopped trapping (T0009) once optimized.

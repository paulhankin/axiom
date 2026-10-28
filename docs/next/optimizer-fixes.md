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
    out wrong;
  - an address with an index such as `x *% 3` (which wraps) was taken never to equal
    another, and a load read a stale value;
  - a load after a call that may exit, at the join of two paths, was moved before the
    call on one of them;
  - a pointer made from an integer, from an address one past the end of an object or
    from an allocator's address plus an offset, was taken for one into that object;
  - a shift by the full width or more of a value chosen by a branch crashed the compiler
    at `-O2`.
- Fixed in x86-64 code at every level: a load moved above a float or vector store to the
  same memory; an AVX-512 masked load above the setting of its mask; a value set before
  a loop that runs no times lost on a path that ends in a trap; a comparison at the start
  of a function's frame setup lost; a shuffle of a 512-bit vector to a 128-bit one on
  processors with AVX2 or AVX-512; an operation reading memory directly
  (`let y = p.* +% 5;`) moved past a later store to that memory (`p.* = 100;`), so it
  read the new value; a 128-bit vector argument passed on the stack after an odd number
  of 8-byte ones read from the wrong place.
- Fixed in x86-64 and AArch64 code at every level: a `match` on an integer with case
  values that do not fit an instruction's immediate (above 2^31 on x86-64, above 65535 on
  AArch64), in a loop or with the value used elsewhere, compared with 0 instead.
- Fixed on WebAssembly: a `match` on a 64-bit integer took a case for a value outside the
  range of its cases, or crashed the compiler when its cases were far apart.
- Fixed: a call through a function pointer cast to another function type crashed the
  compiler at `-O2`, and on WebAssembly stopped trapping (T0009) once optimized.

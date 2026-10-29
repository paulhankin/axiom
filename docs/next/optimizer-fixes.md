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
- Fixed at every level on x86-64 and AArch64: a `match` on a `bool` never took its `true`
  arm; a thread-local's address in a constant aggregate (an array of pointers, a slice
  passed by value) pointed at its initial image, not the thread's copy. On x86-64 a load
  could be moved above an earlier `@atomic_load` (breaking acquire ordering), and with
  AVX2 a vector shift could read a garbage count.
- Fixed at `-O2`: a loop whose header makes a call or a store (`while next() and i < n`)
  could run that header twice in one iteration.
- Fixed on WebAssembly: a value with effects (an atomic operation, a call) computed only
  for one branch's use; vector shifts by a count shared or moved out of a loop;
  `@atomic_cas` without the atomics feature; C variadic calls nested in another's
  arguments; wasm64 without bulk memory; the main thread's stack limit (its lowest KiB is
  reserved, as documented); linking C code into a wasm64 module; and wrong lines in stack
  traces at `-O2`.
- Fixed: `f32` constants were computed in `f64` and rounded once at the end, and decimal
  literals rounded twice (`const D: f32 = K - 16777216.0` with `K = 16777217.0` was 1);
  they are now rounded as `f32` values, each operation as at run time. `std/fmt`'s
  `parse_float` and `parse_float32` gave a result one unit off for some decimals with more
  than 19 significant digits. `@fma` on `f32` without a hardware FMA (x86-64 before v3,
  WebAssembly) rounded some subnormal results twice.
- Fixed on aarch64-macos: stack arguments narrower than 8 bytes are packed at their size,
  as Apple's ABI (and C code calling or called by Axiom) has them.
- `--units=module --format=agent-json` reports errors found during code generation in
  its one JSON document.
- Fixed: a call through a function pointer cast to another function type crashed the
  compiler at `-O2`, and on WebAssembly stopped trapping (T0009) once optimized.

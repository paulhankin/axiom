## Compiler

- The way an error goes is now predicted not taken: the handler of `catch` and of `!`
  after a call, a `return error.E` under a condition, an `errdefer`, and the overflow of
  `@add_checked`, `@sub_checked` and `@mul_checked`. Before, the test of an error code
  (`code != 0`) was predicted true like other tests against zero, so the success path was
  the one laid out out of line and the register allocator weighed the error path as the
  common one. A checked operation tested where it is made (`@mul_checked(x, y) catch
  0`) now branches on the overflow itself, without making the error code first. On a
  benchmark of checked arithmetic and error codes in hot loops
  (`cmd/axiom/bench/checked.ax`) this runs 27.6% fewer instructions. `?T` is unchanged:
  `.some` stays the predicted way of `??`, and `.none` is not treated as an error.
- The marks a front end puts on blocks it knows are seldom run (C's `__builtin_expect`,
  calls of `cold` functions) now survive inlining and the copies loop transformations
  make of blocks.

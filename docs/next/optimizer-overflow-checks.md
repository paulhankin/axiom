## Optimizer

- **Fewer overflow checks, and checks no longer change what is inlined.** At `-O2` an
  overflow check that the ranges of its operands show cannot fail is left out: a
  signed counter below a limit (`i < n` then `i + 1`), `x - 1` where `x > lo`, `-x` and
  `@abs(x)` where `x` is above the minimum, sums of sign- or zero-extended narrower
  values, products of extended 32-bit values, a masked or shifted value plus a constant
  that fits (`(p & ~0xffff) + 128`, `(x >> 1) + 5`), and `(1 << k) - 1` for `k` below the
  width. The inliner no longer counts overflow checks in a function's size, so trapping
  code is inlined as wrapping code is: the allocator's fast paths, which were called
  rather than inlined, cost no more than with `--overflow=wrap`.

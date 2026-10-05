# 0085: Self-hosted optimizer: x - d checks after a test of k - x

- Type: feature (performance)
- Status: fixed (92ab39d)
- Area: compiler (selfhost: gvn)

the benchmark's hitting-shot code (`indirect_double` and the like) does

    let k = top_bit(val);   // 31 - @clz(val)
    if (k <= 23) and (((blot_mask_opp >> @int_cast(u32, 23 - k)) & rel) != 0) { ... }

combine folds the test to `31 - ctlz <= 23` and the shift count to
`ctlz - 8`, and the `@int_cast` check of the count, `ctlz - 8 >= 0`, stayed
(a compare and a branch to a trap, about 30 times in
`hitting_shots_not_on_bar`): gvn's facts are upper bounds, and nothing
turned `31 - ctlz <= 23` into `ctlz >= 8`. LLVM drops the check.

Fix (gvn):
- assume: `k - x <= hi` (signed, or `< hi + 1`) with x in [0, ub(x)] and
  k not negative, so that the subtraction cannot overflow, gives
  `x >= k - hi` (and x != 0) as facts.
- decide: `x - d` (an add of -d) compared, signed, with a constant c >= 0,
  x in [0, ub(x)] (no overflow either way), is `x >= d + c` (`+ 1` for > and
  <=), decided by those facts.

Test: `tests/run/offset_facts.ax` (the the benchmark pattern, the bounded cases
that fold and the ones that do not, and wrapping values that must not).

Effect (the benchmark, `--cpu=x86-64 -O2`): `hitting_shots_not_on_bar` 2246 ->
2060 instructions; bench_kernels `contact_inputs` 497 -> 493 ns
(within the noise).

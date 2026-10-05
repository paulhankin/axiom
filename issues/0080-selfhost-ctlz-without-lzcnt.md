# 0080: Self-hosted x86-64 without LZCNT: k - ctlz(x) and the shifts it bounds

- Type: feature (performance)
- Status: fixed (b16d1ee)
- Area: compiler (selfhost: x64, gvn)

Baseline x86-64 has no `lzcnt`, so `ctlz(x)` was `bsr`, a `cmov` of -1 for
x == 0 and a subtraction from bits - 1 (`mov r,-1; bsr; cmove; mov d,31; sub`),
and the benchmark's `top_bit(m) = 31 - @clz(m)` subtracted again (`mov e,31; sub`).
The hitting-shot features call it for every roll that hits (33 sites in
`hitting_shots_not_on_bar` alone, all under `if m != 0`).

Then `blot_mask_opp >> (23 - k)` (after an `@int_cast` check that 23 - k is
not negative) kept its guard for counts of 32 or more (`cmp; cmovae`): combine
folds `23 - (31 - ctlz)` to `ctlz - 8`, an add of a negative constant, whose
upper bound gvn knew only if `ctlz >= 8` was known.

Fix:
- A ctlz used only as `k - ctlz(x)`, or at 32 bits `ctlz(x) + k` (prepare
  marks it `F_BSR`), holds bsr(x); the users add `k - (bits - 1)` (a `lea`) or
  subtract it from `k + bits - 1`. bsr(0) is taken as -1 (the cmov), dropped
  where a dominating branch shows x != 0 (`nonzero_at`). A generic ctlz of a
  nonzero x is `bsr; xor bits - 1`.
- `ub(y - c)` also holds when the result is known not negative (a dominating
  signed check) and c <= 2^(bits-1): otherwise y - c would wrap to at least
  2^bits - c, a negative number. ub(ctlz - 8) is 24, and the guard goes.

Effect (the benchmark, `--cpu=x86-64 -O2`): bench_kernels `contact_inputs`
588 -> 563 ns.

Left: the `@int_cast` check itself (`23 - k >= 0` right after `k <= 23`
was tested) stays here; LLVM drops it, and 0085 does too.

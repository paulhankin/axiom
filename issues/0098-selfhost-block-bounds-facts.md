# 0098: Self-hosted optimizer: bounds checks of a block after a test that it fits

- Type: feature (performance)
- Status: fixed (0753e8d)
- Area: compiler (selfhost: gvn)

the benchmark's `dim128_pair` (the benchmark `axiom/src/eval/simd_avx512.ax`, the 128-hidden net, a
fifth of a 1-ply search) reads the inputs a 16-float block at a time:

```
let full = ((b * 16) + 16) <= in0.len;
if full {
    m0 = @bitmask(@lanes_ne(@load(f32x16, in0[(b * 16)..]), zero16));
```

Two checks stayed in the loop (about 9 instructions per block, 5k blocks in five 1-ply
positions; LLVM drops them): the slice start `b * 16 <= len` and the vector load's
`(len - b * 16) * 4 >= 64` (bytes). gvn's facts are upper bounds `x <= y + c` keyed by the
value bounded, so `x + 16 <= n` was a fact about the sum, not about `x`; and comparisons
with a constant were decided only by upper bounds, never by a lower bound of `n - x`.

Fix (gvn, 0753e8d):
- `x + k <= n` (or `< n`) with `k` a small constant and the sum not wrapping (`ub(x) + k`
  fits, or `x` below the maximum by `k`) is also the fact `x <= n - k`.
- `a >= k` (and `>`, `<`, `<=` against a constant) is decided by lower bounds:
  `n - x >= k` when `x <= n - k`, and `s * c` (or `s << c`) `>= k` when `s >= k / c`
  rounded up and the product cannot wrap.

Tests: `tests/run/block_facts.ax` (whole blocks and a partial last one, windows of a
slice), `tests/run/block_facts_trap.ax` (a wrapping sum proves nothing: the check stays and
traps).

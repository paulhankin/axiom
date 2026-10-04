# 0060: Self-hosted optimizer keeps shift guards and checks that value ranges rule out

- Type: feature (performance)
- Status: fixed (bd1a230)
- Area: compiler (selfhost: codegen shift lowering, combine, gvn range facts)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).

## Today

**Every variable shift carries two selects.** `codegen.ax` `gen_shift` lowers `x << k`
(where a count of at least the width gives 0) to
`select(k >= w, 0, shl(x, trunc(select(k >= w, 0, k))))`. Nothing removes the selects
when `k` is known to be below `w`. In `generate_moves_sub`'s loop,
`let i = 31 - @clz(mask); mask ^= 1 << i;`, with `i` in 0..=31:

```
IR                                         x86-64 (selfhost)
%111 = icmp.35 i1 %78 32                   cmp    edx,0x20
%112 = select i64 %111 0 %78               mov    ecx,0x0
%113 = trunc i32 %112                      mov    r8,r12
%114 = shl i32 1 %113                      cmp    edx,0x20
%115 = select i32 %111 0 %114              cmovae r8,rcx
                                           mov    ecx,0x1
                                           shlx   ecx,ecx,r8d
                                           mov    r8d,0x0
                                           cmp    edx,0x20
                                           cmovae ecx,r8d
```

LLVM: `mov esi,0x1; shlx eax,esi,r12d`. Even when the count is not known, one
`cmp; shlx; cmovae` suffices: `shlx`/`shrx`/`sarx` mask the count themselves, so the inner
select does nothing on x86-64. The same holds for `half_containment`'s `>> (9 - c)`,
`half_mobility`'s `>> (24 - idx)` and `escapes_index`.

**Checks that ranges prove.** `calculate_half_inputs_with_masks` has 45 calls to
`axiom.trap` in the selfhost's code and 5 in LLVM's. Examples:

| check left in the code | why it cannot fail |
|---|---|
| `lzcnt edx,r9d; cmp edx,0x7fffffff; ja trap` | `@clz` is at most 32 |
| `tzcnt r11d,r10d; cmp r11d,0x19; jae trap` (`accumulate16`) | `cur` is masked by `0x00FFFFFF`, so `@ctz(cur)` is at most 23 when `cur != 0` |
| `cmp r10d,0xd; jae trap` after `@min(idx, 12)` | the result of `min(x, 12)` is at most 12 |
| `cmp r9d,0x7fffffff; ja trap` after an `ESCAPES[...]` load (`@int_cast(i32, ...)`) | every table entry is at most 36 (a constant global) |
| `cmp r8d,0x19; jae trap` (`an_board[idx]` in `half_mobility`) | `idx = 6 + @ctz(mask >> 6 & 0x7FFFF)`, at most 24 |

Each check is a compare and a branch, but the shadow-stack bookkeeping that keeps its
trap path ready also costs (0053). The shift selects also keep the shift amount and the
zero alive.

## Proposal

- `combine.ax` (or the GVN bounds facts in `gvn.ax`, `le_path`/`range_of`): give integer
  values a known range (or known bits): `ctlz/cttz/ctpop` in `[0, bits]`, and
  `[0, bits-1]` for `cttz/ctlz` of a value known non-zero (dominated by `x != 0`, as in
  `while mask != 0`). Also: `and x, c` is in `[0, c]`; `umin(x, c)` is at most `c`;
  `lshr x, c`; `sub c, x` for `x` in a range; `zext` of a narrow value; `select` of two
  ranged values; and loads from a constant global array (the max over its elements, for
  small arrays). Then fold `icmp` against those ranges, which also removes bounds checks
  and `@int_cast` checks.
- `codegen.ax` `gen_shift`: emit the guard once (`select(k >= w, 0, shl(x, k))`, with the
  backend masking the count) and let combine drop it when `k < w` is known. In
  `x64.ax`, a `shl` by a count that is a `select(k>=w, 0, k)` can use `k` directly with
  `shlx`.

## Impact

Five instructions per shift in the movegen loop (two shifts per step) and in the input
kernels' loops, and 40 trap paths with their shadow-stack records in
`calculate_half_inputs_with_masks`. Estimate: 3-6% of movegen and `contact_inputs`,
about 30-50 ns summed.

## Resolution

Fixed in bd1a230, in `codegen.ax` and `gvn.ax` (the bounds facts were already in GVN, so
nothing went into `combine.ax`):

- **Shifts.** `gen_shift` lowers a shift left or logical shift right by a variable count to
  `select(k >= w, 0, op(x, k))`: the count is not clamped (every backend masks it, and the
  select replaces whatever an out-of-range count computes), so an unknown count is
  `shlx; xor; cmp; cmovae` on x86-64. Arithmetic shifts keep their clamped count (the sign
  fill), and constant counts fold as before. GVN removes the select when the count's
  bound is below the width (the movegen loop's `1 << (31 - @clz(mask))` is a lone `shlx`).
- **Upper bounds** (`ub`, used to decide comparisons): bit counts (`ctlz`/`cttz` at most
  `bits - 1` of a value known non-zero, `cttz` below its highest possible bit, `ctpop` at
  most its bit length), `umin`/`umax`, `smin`/`smax` of values known not negative, `and`
  of two variables, `or`/`xor`, `c - y` (less the lower bound `bits - bitlen(ub)` of a
  `ctlz`), `y - z` with `z <= y` known, `y - c` with `y >= c` known, shifts by variable
  counts, `sext`/`ashr` of values known not negative, a select's arms under its
  condition, and loads from constant tables (the largest element; offsets multiples of
  the element size; cached per run). A phi's inputs are taken with the conditions on the
  paths into it (`path_assume`). A loop phi is bounded by induction: the back edge's value
  must stay within the other inputs' bound with the phi assumed within it, using only the
  facts that also hold at the back edge (`fact_lo`/`fact_hi` hide the rest of the scope).
- **Signed facts.** `x > c` (`c >= -1`) or `x >= c` (`c >= 0`) records that `x` is not
  negative and at least `c` as unsigned; then `x <= K` adds an unsigned bound. A signed
  comparison of a value known not negative with a constant that is not negative is decided
  as the unsigned one.

`calculate_half_inputs_with_masks`: 45 trap paths to 4 (LLVM: 5); `generate_moves_sub`:
21 to 16 and 11 selects to 6 (the shifts left with a guard have counts nothing bounds:
`BAR - r` for a byte `r` and `from_pip + 1`). Over `bench_kernels`, 884 trap calls to 810
and 426 selects to 375. Tests: `tests/run/ranges.ax` (shifts by every count 0-299 at each
width and signedness, the patterns above, including values that may be 0) and
`tests/lowering/x64_ranges.ax`.

`bench_kernels` (ns, min of 21, against main at 7fd6c8f): `contact_inputs` 243 to 224
(LLVM 185), `pruned_boards` 64 to 61; the other kernels within noise. `movegen` and
`race_inputs` moved by up to +4% and +8% between builds whose code for those functions is
identical (`calculate_race_inputs` is byte-identical; builds differing only in other
functions' sizes span 860-895 ns in `movegen`): code placement, not the change.

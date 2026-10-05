# 0078: Self-hosted x86-64: idioms left in the benchmark's contact half-inputs (bswap, byte compares, kortest)

- Type: feature (performance)
- Status: fixed (6d3f272)
- Area: compiler (selfhost: x64, combine, thread)

Follow-up to 0071. After it, `contact_inputs` (300 positions, the benchmark
`axiom/src/bin/bench_kernels.ax`, counted by single-stepping) executes 963k instructions
against LLVM's 992k in all, but `calculate_half_inputs_with_masks` alone still runs 490k
against LLVM's 448k. The items of 0071 that are left:

1. **Bit reversal** (0071 item 2): `reverse24` written with five swap stages is 30
   instructions; LLVM uses `bswap` plus three stages. Needs an `ir.IN_BSWAP` intrinsic
   (also for `@byte_swap`) and recognition of the last two stages in combine.
2. **Escapes computed twice** (0071 item 3): `escapes_from_mask` and
   `escapes1_from_mask` both compute `escapes_index(mask, n)` under the same `n <= 0`
   test; jump threading does not merge the two diamonds.
3. **Byte compares not vectorized** (0071 item 4): the six `an_board[i] == 2` tests are a
   `sete`/`shl`/`or` chain where LLVM does one `vpcmpeqb` on a 4-byte load (SLP), and
   the race crossover sums could use `vpsadbw`.
4. **`@any_true`/`@all_true` of 512-bit vectors** (left from 0067) still go through
   `kmovq`, a compare and `setcc` where `kortest` and a jump would do.

Mask registers (0071 item 1) moved to 0077 with the rest of the register allocator.

## Resolution

Fixed in 6d3f272:

1. **Byte swaps** (`combine.ax` `byte_swap`): `rotl(((x >> 8) & 0x00FF00FF) |
   ((x & 0x00FF00FF) << 8), 16)` (either half also masked after the shift; for 64
   bits the 16-bit stage too) is `ir.IN_BSWAP` on the native targets (`bswap`,
   AArch64 `rev`), so a bit reversal by swaps is three stages and a `bswap`. An `or`
   (or `add`) of a value and another shifted left by 1-3 whose bits cannot meet is
   one `lea` (`x64.ax` `F_SCALE`, `maybe_bits`): each stage is five instructions.
   `reverse24`: 30 instructions to 19 (LLVM 19).
2. **Escapes once**: loads from constant globals are never clobbered by stores
   (`gvn.ax` `may_alias`), so the second `escapes_index` is the first; with the
   threading that was there the index is computed once.
3. **Byte compares** (new `slp.ax`): an `or` tree of `zext(load(base + k) pred c) <<
   (s + k)` over neighbouring elements (bytes, words, dwords; at least three; other
   leaves or-ed back) is one load of them (AVX-512: a masked `vmovdqu8`; else
   scalar loads of up to 8 bytes put in a vector), one vector compare and
   `@bitmask`. The six `an_board[i] == 2` tests: 25 instructions to 6.
   `@bitmask(v & c)` for a constant `c` is `@bitmask(v) & bitmask(c)`
   (`simplify_bitmask`): the shot counts read the compare's mask register
   (`vpcmpd k1; kmov; and; popcnt`, no `vmovdqu32`/`vpmovd2m`), as LLVM's.
4. **kortest**: a branch or select on `@any_true`/`@all_true` of a 512-bit vector
   is `vptestmq`/`vptestnm* k1` and `kortestq` (`ptest_cond` for 64 bytes); the
   value form uses the same. This also fixes a bug: `@any_true` of a 512-bit
   vector of 16-, 32- or 64-bit lanes compared the lane mask with all 64 bits set,
   so it was true for a zero vector (`tests/run/idioms.ax`).

Also: `x != 0 ? f(clz(x)) : k` where `f(width) == k` is `f(clz(x))` (and for
`ctz`), with cheap scalar intrinsics now speculated by if-conversion
(`highest_point`: no branch); a compare at the start of a block that every jump
into it (and the code falling in) has just made goes (`reuse_flags_across`: the
`cmp x, 2; jle` / `cmp x, 2; jge` chain of `timing_home_board`); `cmp a, x` after
`mov d, a; sub d, x` goes; a branch on `x == false` of an i1 branches on `x`;
gvn records `x > k` also as `x >= k + 1` (and the like), deciding the
`fast_div_small` range checks.

Counts per 300 `contact_inputs` (single-stepped): 973k to 931k instructions
(LLVM 992k); `calculate_half_inputs_with_masks` 491k to 453k (LLVM 448k). What is
left there is mostly the allocator's: `af_input`, the 8th argument, is reloaded
from its stack slot nine times a call (LLVM keeps it in `r12`) and three
parameters are spilled at entry (measured before 0077's fixes), plus LLVM's two-lane `vplzcntd` for the
back anchors.

Measured (ns per position, min of 11 runs in one session, ±5%; base =
origin/main 37af634, rebuilt on this machine; "fixed" includes 0077):

| kernel | base | fixed | LLVM |
|---|---|---|---|
| contact_inputs | 217.3 | 208.0 | 210.3 |
| race_inputs | 118.3 | 114.0 | 129.6 |
| pruning_inputs | 23.2 | 22.9 | 24.4 |
| movegen | 850.4 | 833.1 | 928.7 |

The net kernels are within the noise.
`bench_search` results (0, 1, 2, 3 ply, rollout) are bit-identical. Tests:
`tests/run/idioms.ax`, `tests/lowering/x64_idioms.ax`.

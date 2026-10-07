# 0069: Self-hosted x86-64: what is left between the input encoders and LLVM's

- Type: feature (performance)
- Status: fixed (7133181, 62d2e71, f8011f3, f40fe96)
- Area: compiler (selfhost: opt layout, x64, mach, gvn, combine)

Follow-up to 0068. After it, `race_inputs` takes about 118 ns per position against
LLVM's 100, `contact_inputs` 192 against 155, `pruning_inputs` 21.7 against 17.5
(the benchmark `axiom/src/bin/bench_kernels.ax`). What the machine code still shows:

1. **Branch layout of equality tests.** In `calculate_race_inputs`' point loop
   (`if nc == 1 {..} if nc == 2 {..} if nc >= 3 {..} if nc > 3 {..}`), the selfhost
   lays each then-block out right after its test (reverse postorder), so an empty point,
   the common case, takes three taken branches; LLVM predicts `==` false and puts the
   stores out of line (one or two taken branches). Writing the stores branch-free took
   the selfhost's kernel from 135 to 91 ns and LLVM's from 105 to 64, so the branches
   are a third of either; how they are laid out is part of the remaining gap.
2. **Redundant compares**: `cmp r8b,0x3 / jb / mov [..] / cmp r8b,0x3 / jbe` — the
   second compare is the same as the first, with only a store between, but in the next
   block (the store's), and `reuse_flags` works within a block.
3. **Constants rematerialised per loop**: after partial unrolling and peeling, the
   constant 1.0 and 0.5 are loaded again in the loop's preheader although an outer value
   holds them (`vmovss xmm3, [1.0]` beside `xmm0`), and `mov r9d,0xf` before each
   `cmova` in `calculate_pruning_inputs`' unrolled loop instead of one register.
4. **Copies in unrolled straight-line code**: `mov rcx,rax / imul ecx,ecx,5` (three
   operand `imul ecx,eax,5`), `mov r9,rdx ... mov rdx,r9` around a `cmov` pair in
   `timing_home_board`.
5. **Byte sums**: sums of `zext(load u8)` times small constants over contiguous bytes
   (`calculate_race_inputs`' crossovers) are scalar chains; LLVM uses `vpsadbw` and
   `vpdpbusd`. Small in these kernels.
6. In `calculate_half_inputs_with_masks`, checks that value ranges rule out (`@int_cast`
   after `popcnt`/`lzcnt`, bounds checks after a `@min`): 0060.

## Resolution

Fixed in 7133181, 62d2e71, f8011f3 and f40fe96:

1. **Static branch prediction for the layout** (`opt.ax` `unlikely_succ`,
   `place_unlikely`). An equality is predicted false (`!=` true), as in GCC and LLVM,
   except a comparison of a boolean (an i1, or a 0/1 value kept in a wider integer: phis,
   selects and zero extensions of them, which is how `not off` reaches a branch) and a
   test of one bit (`x & (1 << k)`), which go either way; a signed integer is predicted
   not negative. After the loops are laid out, an unlikely successor that sits between
   two blocks that could otherwise follow each other without a jump (its branch and the
   likely successor, or an if-else's two arms' join), with the few blocks after it that it
   dominates in the same loop, moves to the nearest place no block falls into (after a
   jump or a return; else the function's end). The register allocator's block
   frequencies weigh the same edges (`MBlock.unlikely`, taken a third of the time). An
   empty point of `calculate_race_inputs` now takes one taken branch, not three: taken
   branches in the kernel 49k to 22k per 300 positions (LLVM 28k).
2. **Constants across loops.** `hoist_constants` reuses the copies the loops around a
   loop made in their preheaders (the 1.0 and 0.5 of the race point loop are no longer
   loaded again after the peeled iterations). A constant a cmov reads (`@min(x, 15)`:
   cmov has no immediate form) is built once per block (`imm_reg`), not before each
   cmov. (Hoisting it out of the loop, as LLVM does, would hold a GPR across the loop;
   GPR constants are not rematerialised, so that was left.)
3. **Copies.** A select pair on one condition compared it twice, and the second compare
   kept the first select's operand alive past its copy, so neither copy coalesced
   (`mov r9,rdx` / `cmovge r9d,..` / `mov rdx,r9` in `timing_home_board`): `reuse_flags`
   now also runs before register allocation. Multiplication by a constant is a three
   operand `imul`; `mov d,a; shl d,1..3` is a `lea`. Compares into `k1` that repeat one
   with nothing between that writes `k1` or their operands are dropped (`reuse_k1`;
   AVX-512 masks all go through `k1`, see 0071).
4. **Byte sums.** Not vectorised (`vpsadbw`, 0071), but `combine.ax` adds the terms of
   one weight first, `p*c + q*c = (p + q)*c` and `(x + p*c) + q*c = x + (p + q)*c`: the
   crossover sums go from 47 instructions a side to 37 (LLVM about 35). Constants are
   also gathered across subtractions (`c1 - (x + c2)`, `c1 - (c2 - x)`, `(c1 - x) +
   c2`: `24 - (12 + (31 - lzcnt))` is one subtraction).
5. **Ranges.** `ub` of `@max(x, c)` with `c >= 0` is the type's signed maximum when
   `x`'s is unknown, so `@min(@max(9 - n, 0), 10)` is bounded by 10 and its `@int_cast`
   and index checks go (the two checks left in `calculate_half_inputs_with_masks` that
   ranges decide; the other two are the `@int_cast` of `moment2`'s product, which LLVM
   keeps too).
6. Also: a constant fill of 256 bytes to 2 KiB stores four vectors per iteration
   (`calculate_race_inputs` zeroes 856 bytes: 104 instructions to 45; LLVM calls
   `memset`).

Not done here: repeated compares in the next block (0067's owner, `thread.ax`/flags).

Counts per 300 positions (single-stepped; instructions / taken branches; base =
origin/main 89872f3, with 0067's fixes): race inputs 237k / 49k before, 212k / 22k after
(LLVM 206k / 28k); `calculate_half_inputs_with_masks` 537k / 17.4k before, 523k / 15.5k
after (LLVM 445k / 13.5k: what is left there is in 0071); pruning inputs 116k to 103k
(LLVM 114k); movegen 1.059M / 132k to 1.049M / 132k.

Measured (ns per position, min of 21 runs in one session, ±5%; base = origin/main
89872f3):

| kernel | base | fixed | LLVM |
|---|---|---|---|
| race_inputs | 100.5 | 83.0 | 91.4 |
| contact_inputs | 176.3 | 159.2 | 143.4 |
| pruning_inputs | 19.3 | 19.2 | 16.3 |
| movegen | 618 | 599 | 591 |

`pruning_inputs` now runs fewer instructions than LLVM's code (103k against 114k) but
stays slower; what is left there is not the instruction count.
The net kernels' code is the same but for register choices and one taken branch per call
in `dim16` (its `if od == 5` fast path is predicted not taken); their timings are within
the noise (which reaches ±8% between runs of one binary on this machine). The compiler
building itself runs as fast as before (within ±3%). `bench_search --mode 2ply` results
are bit-identical. Code size: the benchmark's `bench_kernels` -0.1%, the compiler +0.6%. Tests:
`tests/lowering/x64_layout.ax`, `tests/run/fills.ax`, and cases in `tests/run/ranges.ax`.

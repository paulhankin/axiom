# 0086: Self-hosted x86-64 without SIMD extensions: what is left in the benchmark

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: x64, mach, inline), the benchmark port

Follow-up to 0072, 0073 and 0079-0085. the benchmark's `axiom/` port built with
`--cpu=x86-64 -O2` (the "SIMD off" build) against the LLVM-backed C compiler
at the same flags and the Rust engine at `target-cpu=x86-64`, on a Cascade
Lake machine (bench_kernels, ns per position, min of 9):

| kernel | before 0079 | after 0085 | LLVM | Rust |
|---|---|---|---|---|
| movegen | 983 | 870 | 916 | 961 |
| contact_inputs | 590 | 494 | 442 | 563 |
| race_inputs | 123 | 120 | 131 | 115 |
| pruning_inputs | 26 | 22 | 30 | 99 |
| contact_net | 411 | 385 | 394 | 369 |
| race_net | 314 | 295 | 297 | 272 |
| pruned_contact | 173 | 168 | 180 | 167 |
| contact_single | 452 | 439 | 440 | 433 |
| pruned_boards | 71 | 65 | 65 | 56 |

And bench_search (seconds, min of 3, results identical to Rust's; the
machine is shared, so a few percent is noise):

| mode | before | after | LLVM | Rust |
|---|---|---|---|---|
| 0ply | 0.004 | 0.004 | 0.003 | 0.004 |
| 1ply | 0.049 | 0.041 | 0.038 | 0.037 |
| 2ply | 0.186 | 0.168 | 0.149 | 0.152 |
| 3ply | 1.190 | 1.017 | 0.952 | 0.977 |
| 4ply | 7.50 | 6.56 | 5.94 | 5.80 |
| gbr | 1.75 | 1.61 | 1.26 | 1.29 |
| gbr+ | 3.32 | 3.03 | 2.59 | 2.69 |
| gbr++ | 5.22 | 4.50 | 4.06 | 4.20 |
| rollout | 77.7 | 70.6 | 62.0 | 62.8 |

The selfhost is 1.04-1.24x Rust where LLVM is 0.86-1.03x.

The nets are not compiled for baseline x86-64 at all: the port picks its
kernels at run time (`@target_has` is checked when the program starts), so
on this machine they are the AVX-512 kernels (`simd_avx512.*`, built with
`+avx512f,...`), the same code as in the native build (see 0070, 0071). They
are about 7% behind Rust's even when LLVM compiles them, which points at the
port's code path rather than either compiler. bench_kernels' `contact_net`
is on par with LLVM's, but the searches also use the batched and
incremental kernels (`sigmoid16`, `dim128_pair`, `delta16`,
`pruned_boards16`), which a 3-ply profile (PC sampling) shows as the largest
share of what is left between the selfhost and LLVM at search level, ahead
of `calculate_half_inputs_with_masks`. Those are the AVX-512 code of the
native build, and gain with it.

What is left in the selfhost's own code:

1. **`hitting_shots_not_on_bar` is not inlined** into
   `calculate_half_inputs_with_masks` (LLVM inlines it), so the masks it
   needs are passed through memory and reloaded.
2. **Shifted masks spilled.** In the hitting-shot loops the masks shifted
   by each die (`blot_mask_opp >> k` for several k) live across the whole
   section; the register allocator spills and reloads them where LLVM
   recomputes a shift (one cycle) instead.
3. **`half_point_sums` spills** its accumulators around the inner loop.
4. **Code placement.** Loop heads are not aligned, so an unrelated change
   in code size moves hot loops across 32-byte boundaries: the net kernels
   move by 1-3% when only code before them changes (0084's change to
   `sigmoid_x4` moved `contact_net` from 393 to 399 ns with no change to the
   kernels themselves). Aligning every loop head to 16 bytes as LLVM does
   (padding taking part in branch relaxation) was tried: `contact_net` 399 ->
   387 ns, but `movegen`, `race_inputs` and `pruned_single` 1-5% slower
   (padding run on each entry of inner loops), and aligning only heads not
   entered by falling through gained nothing. Measuring needs a layout-
   independent method first (several builds with shifted code).
5. **The rest of contact_inputs** (494 against LLVM's 442 ns) is also in
   0078 (bit reversal, escapes computed twice, byte compares).

Also seen: in the benchmark's `axiom/` tests, 24 tests that compare with the
Rust engine bit for bit fail at `--cpu=x86-64` with every compiler (the C
compiler and the selfhost, before and after these changes): they check
`@target_has` at compile time to choose which results to expect, while the
kernels are chosen at run time. That is the port's tests, not a compiler
bug.

## After 0106-0112

Items 1 and 3 are done (0110 inlines `hitting_shots_not_on_bar`; 0109 folds
`half_point_sums_simd128`'s constant masks, and its accumulators no longer spill). Also
fixed: lane inserts through the stack (0106), branches for floats chosen between constants
(0108, `race_inputs` now faster than LLVM's and Rust's), the zero test of `bsf` in loops
over set bits (0111), and twelve masks held to the end of their block before being stored
(0112, part of item 2).

Kernels at `--cpu=x86-64` (ns per position, min of 9, loaded machine):

| kernel | before | after | LLVM |
|---|---|---|---|
| contact_inputs | 488 | 458 | 448 |
| race_inputs | 137 | 97 | 114 |

What is left of item 2 is the register allocator's: in `calculate_half_inputs_with_masks`
(both hitting-shot functions inlined) long-lived values used everywhere (the hitters `e`,
the stack argument `blot_mask_opp`) lose their registers to short intervals of higher
weight (reload pieces at the joins of the bar-one loop) and are then reloaded at almost
every use: 545 frame loads in the function against LLVM's 285, with the same number of
instructions and branches executed (callgrind). Evicting them only around the interfering
stretch (region splitting, as LLVM's greedy allocator) is what is needed. Item 4 (code
placement) and 5 stand.

## After 0113 and 0114 (c942ef2)

0113 (548b145) clears the destination of `bsf` / `tzcnt` and the other bit scans, which
waited for the previous iteration's result in the set-bit loops; 0114 (73b2664) builds
`dim128`'s five outputs with four `insertps` and one broadcast rather than sixteen
broadcasts in a row.

bench_search, interleaved min-of-N under `chrt -f 10` (time / min(Rust, LLVM-built port),
the benchmark 3023b3d). `--cpu=x86-64`, before (main at e36c8ad) -> after:

| 0ply | 1ply | 2ply | 3ply | 4ply | gbr | gbr+ | gbr++ | rollout |
|---|---|---|---|---|---|---|---|---|
| 1.105 -> 1.010 | 1.030 -> 0.963 | 1.052 -> 0.987 | 1.036 -> 0.968 | 1.028 -> 0.986 | 1.108 -> 1.040 | 1.074 -> 0.954 | 1.027 -> 1.007 | 1.063 -> 1.005 |

Repeated (12 runs each): 0ply 0.997, gbr 0.962. Every mode is within 2% of the bar or
faster, within the noise of these measurements (the Rust reference itself moves about 3%
from run to run). The AVX-512 build after the same changes: 0ply 1.01-1.04, 1ply 1.067,
2ply 1.051, 3ply 1.118, 4ply 1.002, gbr 1.081, gbr+ 1.080, gbr++ 0.999, rollout 1.048 of
min(Rust, LLVM); there the LLVM-built port is itself 3-13% slower than Rust in most modes
(1ply 1.07, 3ply 1.13), and the selfhost is within about 2% of it except gbr / gbr+
(about 4%, varying from run to run).

Kernels (ns, min of 7, `--cpu=x86-64`, selfhost / LLVM): contact_inputs 455 / 446,
race_inputs 99 / 114, contact_net 369 / 371, race_net 278 / 276, contact_single 418 / 413,
pruned_single 190 / 187, movegen 804 / 916. What is left in `calculate_half_inputs_with_masks`
is still the register allocation described above (reloads of long-lived values).

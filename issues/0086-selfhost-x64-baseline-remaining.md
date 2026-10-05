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

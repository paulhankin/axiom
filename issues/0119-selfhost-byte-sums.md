# 0119: Self-hosted x86-64: sums of neighbouring bytes are scalar loads and adds

- Type: feature (performance)
- Status: fixed (93d01de)
- Area: compiler (selfhost: slp, x64)

the benchmark's `eval_race_bg_with_endgame-table` starts by counting each side's checkers:

    for i in 0..24 { tot_men0 += raw[0][i]; tot_men1 += raw[1][i]; }

Once unrolled, that is 48 byte loads and 46 adds (wrapping at 8 bits), interleaved. LLVM's
SLP vectorizer makes each sum two vector loads summed by `vpsadbw` against zero. In the
self-hosted build this function took 2.2% of a 2-ply search (Zen 4, AVX-512), against 0.6% in
the LLVM-built port; the scalar sums were about two thirds of it. The same shape is
`board.sum_25_checkers` (25 bytes into a `u32`).

Fix: `slp.byte_sums`, after `compare_masks` in each function's pipeline. A root `add` (not
part of a larger add in its block) whose leaves include loads of n >= 8 neighbouring bytes
from one base (`zext` to the sum's width, or the bytes themselves for an 8-bit sum) becomes:
16-byte loads, then the rest in one masked load with AVX-512 (a plain load for exactly 4 or
8 bytes), else in a load of up to 8 bytes with fewer than 4 left as they were; each vector
summed by the new intrinsic `IN_SAD_U8` (`psadbw` against zero, `<2 x i64>` per 16 bytes),
the halves added, truncated to the sum's width (the scalar adds wrap at that width, and so
does a truncation of the exact sum). Other leaves are added back. Nothing may write memory
between the first load and the root.

Effect (Zen 4, interleaved min-of-N, time / min(Rust, LLVM-built port)): 2-ply 1.020 ->
1.008 with AVX-512; within noise elsewhere. In `eval_race_bg_with_endgame-table` what is left
against LLVM is `race_bg_prob` and the `raw[1][18..24] > 0` scans (LLVM: one vector test).

Tests: `tests/run/byte_sums.ax` and `byte_sums_sse2.ax` (results, with and without SIMD
extensions), `tests/lowering/x64_byte_sums.ax` (the psadbw).

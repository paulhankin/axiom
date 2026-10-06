# 0117: Self-hosted x86-64: tzcnt and lzcnt cleared first on processors without the false dependency

- Type: feature (performance)
- Status: fixed (a3bc1cd)
- Area: compiler (selfhost: x64)

0113 clears the destination of every `bsf`, `bsr`, `tzcnt`, `lzcnt` and `popcnt` first
(`mov d, 0`, then the scan reads `d`, so `d` is never the source's register). `bsf` and
`bsr` need it (they keep `d` when the source is zero), and Haswell gives `tzcnt`, `lzcnt`
and `popcnt` the same false dependency; Skylake fixed it for `tzcnt` and `lzcnt` (LLVM's
tuning for Skylake and later sets only `TuningPOPCNTFalseDeps`).

On the benchmark machine (Cascade Lake), four independent `tzcnt rax, rcx` per iteration run
at one per cycle with or without a clear in front of each (0.31 ns), as do `lzcnt` and
`popcnt`; `bsf` waits for the one before (0.92 ns, its latency).

The clears cost a front-end slot each, and the constraint that the result is never the
source's register costs register moves: in the benchmark's `calculate_contact_inputs` (a loop over
the set bits of masks in most of its features) 18,600 of 1,104,801 instructions at 1-ply
were the clears.

Fix (`x64.ax` `bitscan`): with AVX-512 (Skylake server or later, Zen 4: none of them has
the false dependency on `tzcnt` or `lzcnt`) `tzcnt` and `lzcnt` are emitted alone, writing
`d`. `popcnt` keeps its clear, as LLVM's tuning does. Instructions at 1-ply (the benchmark
`bench_search`, five positions): 4,788,981 -> 4,725,499 (-1.3%; `calculate_contact_inputs`
1,104,801 -> 1,084,852, `dim128_pair` 1,091,585 -> 1,073,388).

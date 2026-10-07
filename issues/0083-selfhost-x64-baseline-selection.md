# 0083: Self-hosted x86-64: rel32 jumps, two-micro-op cmovs, and SSE2 details

- Type: feature (performance)
- Status: fixed (32f7a7f, d97dcc9)
- Area: compiler (selfhost: x64, x64enc)

Found comparing the benchmark's input encoders built for baseline x86-64 with LLVM's
code (a traced call through llvm-mca):

1. **Every jump was rel32** (`0f 8x` + 4 bytes, `e9` + 4), however near its
   target: no branch relaxation. 5% of the hitting-shot code's bytes.
2. **cmova / cmovbe / seta / setbe take two micro-ops** (they read CF and
   ZF); the other conditions take one. `@min(nc, 15)` was `cmp; cmova`
   (53k of 1.9M executed instructions in `contact_inputs`), and unsigned `>`
   and `<=` selects likewise.
3. **`(x & (1 << k)) != 0`** was `mov r,1; shl r,cl; test x,r` (bt).
4. **The SWAR popcount** (no POPCNT) loaded its masks with `movabs` into a
   register even at 32 bits.
5. **Vector constants without AVX** were loaded into registers before each
   use: `fold_remat` folded them into instructions only with VEX/EVEX,
   which take unaligned memory, though the constant pool's 16-byte entries
   are aligned as legacy SSE operands must be.
6. **cvtsi2ss/sd without AVX** merged into the destination's previous value
   (a false dependency; with AVX the destination was zeroed first), and so
   did scalar sqrt, cvtss2sd and cvtsd2ss.

Fix: encode_func encodes a function twice when some jumps can be short
(chosen until none changes: as more are short, distances only shrink);
unsigned compares that would end in A or BE compare with k + 1 or the other
way round (and with 0 test NE or E), and unsigned min selects on AE; single
bit tests are `bt`; 32-bit popcount masks are immediates; pool constants fold
into legacy SSE instructions; conversions zero their destination (or work on
a copy of their source). `tests/run/x64_baseline_idioms.ax` checks the
results against the C compiler's.

Effect (the benchmark, `--cpu=x86-64 -O2`): bench_kernels `contact_inputs`
535 -> 521 ns, `pruning_inputs` 26 -> 23 ns; code about 4% smaller.

With unsigned compares against 0 now NE/E, 0070's `dup_exit_tests` dropped
more tests at the start of loop-exit blocks, exposing that `peephole`
assumed the flags dead at the end of every block: it turned the `mov r, 0`
and `sub` before such a dropped test into `xor` and `lea`, and std/unicode's
`prev_start` loop exited on the xor's flags. Fixed first (3178090): peephole
starts with the flags live at the end of a block whose successor's test was
dropped.

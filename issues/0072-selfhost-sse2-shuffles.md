# 0072: Self-hosted x86-64 without AVX: constant shuffles through memory

- Type: feature (performance)
- Status: fixed (b2dccf2)
- Area: compiler (selfhost: x64)

On baseline x86-64 (`--cpu=x86-64`, SSE2), and with SSSE3 but no AVX, `shuffle_match`
(the shuffles that are one instruction: `punpck*`, `pshufd`, `pmovzx*`) required AVX, so
every constant shuffle that `pshufb` could not do went through memory: both operands
stored, the lanes copied one at a time, the result loaded (a store-forwarding stall).
the benchmark's `half_point_sums_simd128` widens bytes to words (`@shuffle(v, zero, [0, 16, 1, 16,
...])`, four times per side) and `board_hash_words` takes the even bytes of two vectors;
with the AVX-512 net kernels chosen at run time, the input features are the larger part of
the baseline build's gap to LLVM's.

Fix: the 128-bit matches apply from SSE2 (`pmovzx*` only with SSE4.1; without it a zero
extension is `punpckl*`/`punpckh*` with the zero vector, any of whose lanes will do), and
the even or odd 8- or 16-bit lanes of two vectors are a pack: `pand` (dropped when the
other halves are known zero: an and with a constant) or `psrlw` then `packuswb`;
`psrad` (odd) or `pslld`+`psrad` then `packssdw`, or `pand`+`packusdw` with SSE4.1.
`tests/run/simd_shuffle_sse2.ax` checks them against the C compiler at baseline x86-64.

Effect (the benchmark, `--cpu=x86-64 -O2`): bench_kernels `contact_inputs` 716 -> 471 ns
(LLVM-backed 330, Rust 410); bench_search 1ply 1.52 -> 1.20 of Rust's time, 2ply 1.51 -> 1.32.

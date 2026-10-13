# 0121: Self-hosted x86-64: with AVX2 only, a 512-bit vector's halves are joined in a zmm register

- Type: bug (wrong code: AVX-512 instructions for a processor without them)
- Status: fixed
- Area: compiler (selfhost: vsplit)

With AVX2 (`--cpu=haswell`, x86-64-v3) a 512-bit vector is computed as two 256-bit halves
(`vsplit.ax`), and `@reduce_add` of one starts by adding its two halves, read through part
shuffles of the whole vector. vsplit kept every join or part shuffle ("plumbing", which a
target with wide registers always selects) whatever its width, so the part shuffles kept
reading the 512-bit vector whole, and the halves were joined into a zmm register only to be
taken apart again:

    vinserti64x4  $0x1, %ymm1, %zmm0, %zmm0
    vextracti64x4 $0x1, %zmm0, %ymm1
    vaddps        %ymm1, %ymm0, %ymm0

Both are AVX-512F (EVEX) and fault on a Haswell; a 128-bit part of a 512-bit vector came
out as `vextracti32x4` the same way. Found in the benchmark's `bench_search` built for
`--cpu=haswell`: `Evaluator.prune2_scores` (the Prune2Net head reduces an f32x16) was the
only function outside the `+avx512f` ones with EVEX code, and valgrind (which has no
AVX-512) stopped on it.

Fix: plumbing is kept only as wide as the target's vectors. A wider part is the operand's
pieces of its width (taken from its own pieces: no instruction for a half, `vextracti128`
for a quarter); a wider join is the pieces of its two halves. The pieces nothing reads are
removed after splitting.

Test: `tests/lowering/x64_wide_halves.ax` (f32x16 `@reduce_add`, 128-bit parts of an f32x16
and two f32x8 joined, at `--cpu=haswell`: no zmm, mask or xmm16-31 register, no
`vinserti64x4`/`vextracti64x4`/`vextracti32x4`).

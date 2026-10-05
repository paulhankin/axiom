# 0090: Self-hosted x86-64: reductions of table loads under a condition (the benchmark's Zobrist hash)

- Type: feature (performance)
- Status: in progress
- Area: compiler (selfhost: combine, slp)

the benchmark's `compute_board_hash` (the benchmark `axiom/src/core/moves.ax`, the same as the
Rust engine's `moves.rs`):

```
for side in 0..2 {
    for pt in 0..25 {
        let c = raw[side][pt];
        if c > 0 and c <= 15 { h ^= ZOBRIST[(side * 25 + pt) * 16 + c]; }
    }
}
```

In `bench_kernels`' movegen kernel (100 calls of `generate_moves_into`, counted by
single-stepping) `generate_moves_into` runs 79.9k instructions built by the self-hosted
compiler against 32.1k from the LLVM-backed C compiler. LLVM's loop vectorizer makes the
inner loop four lanes wide: a masked byte compare into `k1` and a masked
`vpgatherdq`/`vpgatherqq` per four points (12 gathers a call), then an `xor` reduction.
The self-hosted compiler has no loop vectorizer; it unrolls the inner loop by four, and
if-conversion turns each step into ten instructions: the load speculated through
`select(c, &ZOBRIST[i], &ZOBRIST[0])` (`thread.ax` `spec_load`), then
`h = select(c, h ^ load, h)`, two instructions on the reduction's chain.

The question was whether to vectorize such reductions too (an SLP of the unrolled
code or a loop vectorizer), with masked gathers on AVX-512.

## Gathers are slow on this machine

The benchmark machine (Xeon @ 2.80GHz, Cascade Lake: AVX-512 F/DQ/CD/BW/VL/VNNI)
runs gathers in microcode, as Skylake-SP to Tiger Lake do with the Gather Data Sampling
(Downfall) mitigation, which a guest cannot see (`/sys/.../gather_data_sampling` says
"Not affected"). Measured with independent gathers in a loop (throughput):

| instruction | cycles each | per element |
|---|---|---|
| `vpgatherdq ymm{k}` (4 qwords) | 29-31 | 7.5 |
| `vpgatherdq zmm{k}` (8 qwords) | 25-30 | 3.4 |
| scalar `xor r, [table + i*8]` | ~1 | 1 |

So the vector form loses here, whatever the width:

- a hash of 50 points in a loop over 64 boards: LLVM's gathers 106-119 ns a hash,
  the self-hosted scalar code 44-53 ns;
- the benchmark movegen (ns per position, min of 9): self-hosted 835, LLVM 936, the same LLVM
  build with `+prefer-no-gather` 887, Rust 841. Rust (LLVM 21, `target-cpu=native` =
  `cascadelake`) emits no gather here: newer LLVMs tune Skylake-SP and its successors
  with `prefer-no-gather`; LLVM 18, which the C compiler uses, does not (issue 0091).

Vectorizing these reductions with gathers would make the self-hosted code slower on
the machine it is measured on, so this issue does not add it: it should come with a
per-CPU cost model (fast gathers: Zen 4, Sapphire Rapids and later Intel cores;
`prefer-no-gather` for Skylake-SP through Tiger Lake), and is worth doing only for
targets that have them. Nothing cheaper exists for SSE2/AVX2 either: the lanes index
different 16-entry tables, so no in-register table lookup (`vpermt2q`) applies.

## What helps the scalar form

If-conversion loads from the table's start where the condition fails. When that
element is the identity of the reduction (`ZOBRIST[0]` is 0: count 0 has key 0),
`select(c, h ^ load(select(c, &T[i], &T[0])), h)` is just
`h ^ load(select(c, &T[i], &T[0]))`: one instruction on the reduction's chain
(`xor h, [addr]`) and one fewer `cmp`/`cmov` per element.

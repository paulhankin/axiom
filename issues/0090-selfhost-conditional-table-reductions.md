# 0090: Self-hosted x86-64: reductions of table loads under a condition (the benchmark's Zobrist hash)

- Type: feature (performance)
- Status: fixed (bf39971)
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

## Resolution

Fixed in bf39971 (`combine.ax` `spec_reduction`): `c ? h op f(T[i]) : h` where if-conversion
loads `T[i]` from `c ? &T[i] : &T[0]` and `f(T[0])` is the identity of `op` (`add`,
`sub`, `or`, `xor`, `and`, `mul`; integers only), is `h op f(load)`; likewise
`c ? f(T[i]) : k` when `f(T[0])` is `k`. `f` is up to four casts and operations
with a constant (`T[c] << k`). When `T[0]` does not do, an element of the table
that does (among the first 1024) is loaded instead. the benchmark's hash is then seven
instructions a point (`movzx; lea; lea; lea; cmp; cmovae; xor h, [p]`) with one
instruction on the chain, and the smaller body lets the inner loop be unrolled fully.

Counts per 100 `generate_moves_into` (single-stepped): 79.9k to 59.7k instructions
(LLVM 32.1k, but twelve of its instructions a call are microcoded gathers).
The hash alone (50 points, 64 boards in turn): 44 to 30 ns (LLVM 106).

Measured (ns per position, min of 11 runs in one session, ±5%):

| kernel | base | fixed | LLVM | Rust |
|---|---|---|---|---|
| movegen | 836.4 | 817.0 | 934.0 | 835.7 |

The other kernels are unchanged (no other function's code changes). `bench_search`
(50 positions, min of 7-15 runs, us a position): 1-ply 188.6 / 188.9 (LLVM 187.8),
2-ply 2493 / 2493 (LLVM 2765): the hash is too small a part of a search to show.

`bench_search` results (1 and 2 ply) are bit-identical. Test: `tests/run/table_reductions.ax`.

Left: the address of each point is `lea t, [side_base + c]; lea p, [Z + t*8 + 128k]`;
with `Z + side_base*8` computed once (address reassociation) it would be one `lea`
(50 instructions a call). A gather vectorizer remains possible for CPUs with fast
gathers, with a per-CPU cost model (see above).

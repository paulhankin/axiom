# 0021: Self-hosted compiler crashes splitting wide vectors (fill_phis)

- Type: bug
- Status: fixed (04f5fc3)
- Area: compiler (selfhost)

## What happens

Building the benchmark's Axiom port natively with the self-hosted compiler
crashes; the C compiler builds the same program, and the self-hosted one
builds it for wasm32:

```
$ selfhost/build/axiom build src/bin/bench_kernels.ax --target=native -O2 -o bk
axiom: crash: segmentation fault (address 0x3500000008)
stack trace (innermost first):
  at List.[*mut Value].get
  at fill_phis (selfhost/src/vsplit.ax:699:39)
  at split_wide (selfhost/src/vsplit.ax:790:5)
  at emit_module (selfhost/src/backend.ax:46:29)
  at codegen_program (selfhost/src/codegen.ax:4747:12)
```

(at b6f5443; run from the benchmark's `axiom/` directory). The program's hot
code keeps `f32x16` accumulators live around loops (`eval/net.ax`'s
`dim128_wide`, and since then `eval/simd_avx512.ax`), so a phi of a wide
vector type whose split pieces are not all created is the likely cause.

## Resolution

vsplit keeps its per-value state (`Rep`) in `Value.auxp`, which inlining, loop splitting (gvn), threading and outlining also use for the clones they make, without clearing it. A phi with such a stale pointer looked split, and `fill_phis` read the first fields of the clone (`kind`, `op` of a phi: 0x35_00000008) as a `Pieces` pointer. `split_wide` now clears `auxp` of every instruction before it starts. (std/graphics/shape.ax and webp.ax had such stale pointers too.)

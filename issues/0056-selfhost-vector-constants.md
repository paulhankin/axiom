# 0056: Self-hosted x86-64: vector constants reloaded at every use, splats not broadcast

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: x64 instruction selection, opt hoisting)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).
Mostly the net kernels (`contact_net`, `race_net`, `*_single`, `pruned_*`), where
`sigmoid16` (`src/eval/sigmoid.ax`) is inlined 17 times into `dim128_pair` and once per
board in the pruned kernels.

## Today

`x64.ax` `remat` treats every splat of a constant and every load from a constant global
as free: it is loaded again from memory at each use (0046, to keep 512-bit functions
from holding constants through loops). This applies even when registers are free and
the same constant is used dozens of times:

- **Tables reloaded per inlined copy.** `sigmoid16`'s eight `E_TABLE_128` vectors are
  loaded again in each of the 17 copies in `dim128_pair` (8 x 17 = 136 64-byte loads per
  pair of positions). LLVM loads them into zmm22-29 once, outside the whole batch loop.
- **The same splat loaded repeatedly in one expression.** `half_point_sums_avx512` loads
  the lane-index constant `point` 8 times in 30 instructions:
  ```
  vmovdqu64 zmm19,ZMMWORD PTR [rip+...41b7c0]   ; point
  vpcmpnlew k1,zmm19,zmm17
  vmovdqu64 zmm20,ZMMWORD PTR [rip+...41b7c0]   ; point again
  vpaddw  zmm20,zmm20,ZMMWORD PTR [rip+...41b800]
  vmovdqu64 zmm21,ZMMWORD PTR [rip+...41b7c0]   ; and again
  ...
  ```
- **Zero from memory.** `@splat(0.0)` is a 64-byte constant-pool load rather than
  `vpxord z,z,z`: `dim128_pair`'s mask loop (`vmovdqu64 zmm25,[zero]; vcmpneqps
  k1,zmm24,zmm25`), and a vector `@mem_set` emulation that stored 14 zmm zeros loaded the
  zero 14 times.
- **Splats are full-width constants, never broadcasts.** LLVM uses `vbroadcastss
  zmm,DWORD PTR [rip+c]` once, hoisted, or an embedded broadcast
  `vpord xmm2,xmm2,DWORD BCST [rip+c]`. The selfhost stores 64 bytes per splat in the
  constant pool and loads the whole vector.
- **Broadcast of a scalar from memory goes through a register.** `@splat(p.*)` in the
  FMA loops of `dim128`, `dim128_pair` and `delta16` is `vmovss xmm8,[rsi+rbx*4];
  vpbroadcastd zmm8,xmm8`: a port-5 shuffle that competes with the FMAs (2 per 16 FMAs
  in `dim128_pair`). LLVM uses `vbroadcastss zmm5,DWORD PTR [rdx+r9*4]`, which issues on a
  load port only.
- **Scalar float constants** go through a register: `vmovss xmm14,[rip+c]; vdivss
  xmm0,xmm0,xmm14` instead of `vdivss xmm0,xmm0,DWORD PTR [rip+c]`.
- **A constant not in the r/m slot** is loaded into a register, e.g. `vmovdqu64
  zmm26,[ten]; vcmpltps k1,zmm25,zmm26` (instead of swapping the predicate and using
  memory). LLVM also turns `@select(|x| < 10, |x|, 10)` into `vminps`.
- **`@fma(1.0, w, h)`** (`accumulate16`/`accumulate8`, once per occupied point) loads
  `1.0` and does an FMA. LLVM emits `vaddps zmm6,zmm4,[w]`. This one is a combine, not
  isel: `fma(1, x, y)` is exactly `x + y`.

Instruction counts per `sigmoid16`: selfhost about 55 (16 of them constant loads), LLVM
about 30 with every constant in a register. In the pruned kernels' inner loop
(`accumulate16`), the selfhost has about 30 instructions per point and LLVM 13.

## Proposal

- Rematerialize only under register pressure: keep a constant in a register (one load,
  hoisted to the entry or the loop preheader by `opt.ax` `hoist_constants`) when the
  allocator has room, and rematerialize only what it would otherwise spill. A simple
  version: CSE constant loads and splats within a block/function, hoist them, and let
  the allocator rematerialize instead of spilling (`mach.ax` would need a remat flag on
  the interval rather than a stack slot).
- Splat of zero (and all-ones) is `vpxord`/`vpternlogd 0xff`, never a load.
- Splat of a scalar constant: a 4/8-byte constant with `vbroadcastss`/`vpbroadcastd` (or
  EVEX embedded broadcast `{1to16}` on the r/m operand of arithmetic), which also
  shrinks the constant pool 16x.
- `@splat(load)` is `vbroadcastss zmm, m32` (the load folded).
- Scalar float constants as memory operands of `vaddss`/`vmulss`/`vdivss`/`vcmpss`.
- Commute compares (swap the predicate) and commutative operations so the constant is the
  r/m operand. Recognise `select(x < c, x, c)` as `vminps`.
- combine: `fma(1.0, x, y) -> x + y` (exact).

## Impact

In `dim128_pair`, PC samples on the sigmoid part are about 270 against LLVM's 150 (of
1230 and 832 samples for the whole function). That is about 10% of `contact_net`. The
pruned kernels' per-point loop is twice LLVM's length. Estimate: 5-10% on
`contact_net`/`race_net`/`contact_single`, 10-20% on `pruned_*`, about 50-80 ns summed.

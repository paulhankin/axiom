# 0056: Self-hosted x86-64: vector constants reloaded at every use, splats not broadcast

- Type: feature (performance)
- Status: fixed (459a664, 8f61b1f)
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

## Fixed

In 459a664 (constants), with the rest in 8f61b1f (masks and shuffles, 0058 and 0062):

- **Rematerialization in the allocator, not at every use.** A splat of a constant, a
  load from a constant global, and a constant operand (once per block, up to a local
  label) are values defined once. The definition is registered with `mc.set_remat`;
  when the allocator would spill such a value it rematerializes it instead: in each
  block that reads it as a value of its own (which can be rematerialized in turn), or,
  when it is read only in its own block, at each use, folded into the instruction's r/m
  operand where the target can (`mc.Target.fold_remat`). No stack slot, no stores. A
  constant read once in the block defining it is read from memory there
  (`fold_remats`). A copy of a constant, and a bitcast of any vector, share its register.
- **Zero and all ones** are `vpxor`/`vpcmpeqd`/`vpternlogd 0xff`, never loads.
- **Splats** of 32- and 64-bit constants are `vpbroadcastd/q` of a 4- or 8-byte constant
  (with AVX2); read once by an EVEX instruction they are an embedded broadcast operand
  (`vpandd zmm, zmm, DWORD BCST [rip+c]`), else a full-width constant made on demand.
- **`@splat(load)`** is `vpbroadcastd zmm, DWORD PTR [rsi+rbx*4]` (8f61b1f), and a commutative
  vector operation takes a folded load as either operand (`vaddps zmm, zmm, [w]`).
- **Scalar float constants** are memory operands when read once (the same folding).
- **Compares** put a constant in the r/m slot (the predicate swapped), 8f61b1f;
  `select(a < b, a, b)` is `vminps`/`vmaxps` (8f61b1f, 0058).
- **combine:** `fma(1, x, y)` is `x + y` (8f61b1f).

Also found and fixed on the way: `vpmovm2*` has a false dependency on its destination
(on this Sapphire/Emerald Rapids machine): once constants stayed in registers, the
expansion of each sigmoid's compare into a vector waited for the last sigmoid's result,
which chained the 17 copies in `dim128_pair` (a micro-benchmark of 16 sigmoids went from
220 to 330 ns). The destination is now zeroed first (an idiom with no dependency), and
since 8f61b1f compares rarely need expanding at all.

`sigmoid16` is now about 33 instructions (from 55): the table pairs in registers
where they fit (two of the four kept, the other two loaded into the permute's
destination, as the permute destroys it), constants broadcast or in registers.
`dim128_pair` went from 1715 to 1363 instructions (constant loads 278 to 44).

What is left: `dim128_pair` still reloads the `-beta` splat and the activations from the
stack in the sigmoid and output loops (0054), and holds only part of the table in
registers.

Measured with 0058 and 0062 (`bench_kernels`, native `-O2`, min of 30 runs alternating,
built by origin/main 51b2245 and by this change, on a machine with a load average of 8-10
on 4 cores, so differences of a few percent are noise; checksums unchanged, `bench_search`
1-, 2- and 3-ply results bit-identical):

| kernel | before | after | C compiler |
|---|---|---|---|
| contact_inputs | 297 | 244 | 181 |
| contact_net | 386 | 344 | 288 |
| race_net | 262 | 230 | 196 |
| contact_single | 374 | 350 | 305 |
| pruned_contact | 111 | 99 | 88 |
| pruned_race | 135 | 136 | 117 |
| pruned_single | 152 | 142 | 126 |
| pruned_boards | 79 | 69 | 53 |
| movegen, race_inputs, pruning_inputs, classify | 971, 194, 26, 6 | 985, 198, 27, 5 | 723, 104, 18, 5 |

A micro-benchmark of 16 `sigmoid16` calls on independent vectors: 224 ns before, 115 ns
after.

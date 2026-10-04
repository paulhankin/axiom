# 0058: Self-hosted x86-64: AVX-512 compares are expanded to vectors instead of used as k-masks

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: x64 instruction selection, combine)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).
Issue 0046 listed "`@select` through `k` masks (`vblendm*`) instead of expanded lanes"
and "`@any_true`/`@all_true` of compares via `vptestm`" as remaining. This issue gives
the cases that matter in the benchmark.

## Today

**Compare & value -> expand, then `and`.** `half_point_sums_avx512`
(`src/eval/features.ax`), `@lanes_gt(point, back) & ((point + 1) - back)`:

```
selfhost                                  LLVM
vpcmpnlew k1,zmm19,zmm17                  vpcmpgtw k1,zmm2,zmm0
vpmovm2w  zmm19,k1                        vpsubw   zmm5{k1}{z},zmm4,zmm0
vpaddw / vpsubw zmm21,...
vpandd    zmm19,zmm19,zmm21
```

**`@select(m, a, b)` -> expand + `vpternlogd 0xca`.** In `sigmoid16` (3 selects) and
`half_point_sums_avx512`:

```
selfhost                                   LLVM
vcmpltps  k1,zmm25,zmm26                   vminps zmm9,zmm9,zmm1            ; select(|x|<10,|x|,10)
vpmovm2d  zmm26,k1
vpternlogd zmm26,zmm25,zmm27,0xca

vcmpleps  k1,zmm25,zmm16                   vcmpleps k4,zmm1,zmm8
vpmovm2d  zmm16,k1                         vmovaps  zmm31{k4},zmm18         ; merge-masked move
vpternlogd zmm16,zmm25,zmm26,0xca
```

**Mask of `(x & c) != 0` -> `and`, compare with a zero loaded from memory, expand.**
`sigmoid16`'s `@lanes_ne(idx & 32, 0)`:

```
selfhost                                   LLVM
vpandd    zmm28,zmm26,[c32]                vptestnmd k1,zmm10,zmm25
vmovdqu64 zmm1,[zero]                      (then vmovdqa32 zmm11{k1},zmm26)
vpcmpnequd k1,zmm28,zmm1
vpmovm2d  zmm28,k1
```

**Unsigned byte compare against zero via sign flips.** `bd.compute_occupied_mask`
(`@bitmask(@lanes_gt(v, zeros))` on `u8x16`, inlined into movegen, the pruned kernels and
every input kernel):

```
selfhost                                   LLVM
vmovdqu xmm5,[zeros]                       vptestmb k0,xmm4,xmm4
vpxor   xmm3,xmm3,[0x80 x16]               kmovw    r11d,k0
vpxor   xmm5,xmm5,[0x80 x16]   ; 0 ^ 0x80, a constant computed at run time
vpcmpgtb xmm3,xmm3,xmm5
vpmovmskb r9d,xmm3
```

In the pruned kernels that is 2x per board side.

## Proposal

In `x64.ax`, with AVX-512 (VL/BW for the narrower lanes):
- `and(sext(cmp), x)` / `select(cmp, x, 0)` -> the operation producing `x` with `{k}{z}`, or
  `vmovdqa32/vmovaps x{k}{z}`. `select(cmp, a, b)` -> `vblendmps`/`vpblendm*` with `k`, or a
  merge-masked move or operation when `b` is already in the destination. A compare
  feeds `vpmovm2*` only when its value is needed as a vector.
- `icmp ne/eq (and x, c), 0` -> `vptestm*`/`vptestnm*` into `k`. `icmp ugt x, 0` ->
  `icmp ne x, 0` in `combine.ax`, for vectors as for scalars. Unsigned vector compares
  -> `vpcmpu*` into `k` rather than the SSE sign-flip emulation, and constant-fold the
  emulation's `xor` of two constants where it remains (SSE/AVX2).
- `select(fcmp olt x, c; x; c)` and the like -> `vminps`/`vmaxps` (combine), where the
  NaN behaviour matches.
- `@bitmask` of a `vptestm`/`vpcmp` result -> `kmov` (already done for compares since 0046;
  extend it to the `ptest` forms).

## Impact

About 10 instructions per `sigmoid16` and 4-6 per `half_point_sums_avx512` and
`compute_occupied_mask`. Estimate: 3-6% of the net kernels, 5-10% of `pruned_*`, a few
percent of `contact_inputs`; about 30-50 ns summed. It overlaps 0056 in `sigmoid16`: the
two together take its 55 instructions to about 30.

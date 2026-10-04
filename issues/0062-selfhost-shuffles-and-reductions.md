# 0062: Self-hosted x86-64: constant shuffles as table permutes, lane extracts and reductions

- Type: feature (performance)
- Status: open
- Area: compiler (selfhost: x64 instruction selection)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).

## Today

**Two-source constant shuffles become `vpermt2*` with an index vector from memory.** Since
0046, every wide shuffle that is not a widening is one `vpermt2d/q/ps/b` by a constant
index. The index is loaded from memory, and the instruction overwrites one source, so
there is also a `vmovaps` copy. `sum_lanes_i32_x8` (`half_point_sums_avx512`) and
`sum_lanes_x8` (`output_x8`) are written as the in-lane interleaves that Rust's
`_mm512_unpacklo_epi32`/`_mm512_shuffle_i32x4` give:

```
selfhost                                         LLVM
vmovdqu64 zmm23,[rip+idx0]                       vpunpckldq zmm8,zmm6,zmm7
vmovaps   zmm24,zmm20                            vpunpckhdq zmm6,zmm6,zmm7
vpermt2d  zmm24,zmm23,zmm21                      vpaddd     zmm6,zmm8,zmm6
vmovdqu64 zmm23,[rip+idx1]
vpermt2d  zmm20,zmm23,zmm21
vpaddd    zmm20,zmm24,zmm20
```

and at the 128-bit-group steps, `vpermd` by a loaded index where LLVM uses `vshufi64x2`
with an immediate. On SKX-class cores `vpermt2d` is 3-cycle latency on port 5, and
`vpunpck*` is 1 cycle.

**Zero-extension written as a shuffle with zero becomes `vpermt2b`.**
`half_point_sums_avx512` widens `u8x64` lanes 0..32 to `i16x32` with
`@shuffle(bytes, zero, [0, 64, 1, 64, ...])`. Selfhost: two index/zero loads and
`vpermt2b` (VBMI). LLVM: `vpmovzxbw zmm1,ymm0`.

**Lane extraction.** `@extract_lane(sums, k)` for k = 0..5:

```
selfhost                                         LLVM
vmovaps xmm14,xmm17 ; vpextrd ebx,xmm14,0x0       vmovd   r8d,xmm0
vmovaps xmm14,xmm17 ; vpextrd eax,xmm14,0x1       vpextrd eax,xmm0,0x1
vextracti32x4 xmm14,zmm17,0x1 ; vpextrd r12d,xmm14,0x0
vextracti32x4 xmm14,zmm17,0x1 ; vpextrd r13d,xmm14,0x1   (one vextracti128, reused)
```

The copy to `xmm14` happens because the selector uses the VEX form, which cannot name
xmm16-31. EVEX `vpextrd` can. Lane 0 should be `vmovd`.

**`@reduce_add` on `f32x16`** (`dim128_pair`'s output loop, 10 per pair):
`vmovaps ymm2,ymm24; vextracti64x4; vaddps ymm; vmovaps xmm4,xmm2; vextracti128;
vaddps xmm; vmovaps; 3x vpshufd; 3x vaddss` is 13 instructions. LLVM uses 8:
`vextractf64x4; vaddps; vextractf128; vaddps; vshufpd; vaddps; vmovshdup; vaddss`. The
order of the last two additions has to follow spec §13.2 (0028: fold the halves), which
the `vshufpd`/`vmovshdup` form does.

## Proposal

In `x64.ax`'s shuffle selection, before falling back to a table permute:
- match `vpunpck{l,h}{bw,wd,dq,qdq}`/`vunpck{l,h}ps/pd` (per 128-bit lane interleaves),
  `vshufps`/`vpshufd` with immediates, `vshuf{i,f}{32x4,64x2}` (128-bit groups),
  `valignd/q`, `vpslldq/vpsrldq`, and a shuffle with a zero vector that is a zero
  extension (`vpmovzx*`) or a byte shift;
- when a table permute is still needed, prefer the single-source `vpermd/vpermps` form, and
  hoist the index constant (0056).

Extract lanes with EVEX `vpextrd/q`, `vmovd/q` for lane 0, and extract a 128-bit group
once for several lanes. Use the `vshufpd`+`vmovshdup` tail for float reductions.

## Impact

About 20 instructions in each `half_point_sums_avx512` (twice per contact position), the
output layers' reductions, and `output_x8`. Estimate: 2-4% of `contact_inputs` and 1-3%
of the net kernels, about 10-20 ns summed.

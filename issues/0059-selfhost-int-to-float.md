# 0059: Self-hosted x86-64: int-to-float conversions go through the stack; float division by powers of two

- Type: feature (performance)
- Status: fixed (68528df)
- Area: compiler (selfhost: x64 instruction selection, combine)

Part of the set 0053-0064 (the benchmark `bench_kernels`, selfhost against LLVM; see 0053).
This one is small to fix and measurable.

## Today

**`cvtsi2ss` reads its destination, so the register allocator keeps it live and spills
it.** `x64.ax` `cvt_si2f` emits the AVX form `vcvtsi2ss d, d, r` with `d` as a use-def
(`RUD`), so `d` is live before the conversion (from nowhere, or from an unrelated
earlier value). In `calculate_half_inputs_with_masks` this becomes a 16-byte stack round
trip around each of its 26 conversions:

```
vmovdqu xmm0,XMMWORD PTR [rbp-0x100]
vcvtsi2ss xmm0,xmm0,rax
vmovdqu XMMWORD PTR [rbp-0x100],xmm0
vmovss  xmm0,DWORD PTR [rip+c]         ; 36.0 (issue 0056: should be a memory operand)
vmovdqu xmm1,XMMWORD PTR [rbp-0x100]
vdivss  xmm0,xmm1,xmm0
```

Where it is not spilled, it is still a false dependency on the register's previous
value (`vcvtsi2ss xmm0,xmm0,r12` in `calculate_race_inputs`). LLVM uses a register
zeroed once for the whole function (`vcvtusi2ss xmm4,xmm6,edi`, xmm6 never written).

**`u32 -> f32` is a zero extension and a 64-bit signed conversion.** AVX-512F has
`vcvtusi2ss`, which LLVM uses (and with a memory operand:
`vcvtusi2ss xmm0,xmm8,DWORD PTR [rdx+rax*4]` for `@float_from_int(f32, ESCAPES[af])`).

**Division by a power of two is a division.** `@float_from_int(f32, nc - 3) / 2.0`
(`calculate_race_inputs`, up to 46 times per position) and `/ 4.0`, `/ 8.0` in
`half_backgame` are `vdivss` (latency 11, throughput 3-5). LLVM emits `vmulss` by the
exact reciprocal.

## Measured

Patched `cvt_si2f` in a scratch copy of the compiler to emit `vzero(d)` before the
conversion, so `d` is not live in (a two-line change), then built `bench_kernels` with
it (min of 7, ±5% noise):

| kernel | selfhost | patched | LLVM |
|---|---|---|---|
| contact_inputs | 338 | 307 | 184 |
| race_inputs | 209 | 197-200 | 101 |
| contact_single | 378 | 346 | 279 |

The conversions now read `vpxor xmm0,xmm0,xmm0; vcvtsi2ss xmm0,xmm0,eax; vdivss ...`,
with no stack traffic.

## Proposal

- `x64.ax` `cvt_si2f`: break the dependency. Either zero `d` first (`vzero`, which reads
  nothing), or, better, pass one register zeroed once in the function as the merge source
  (`vcvtsi2ss d, zreg, r`, `zreg` read-only). Do the same for the other scalar AVX forms
  that merge into the destination (`sse_unary` for `cvtss2sd`/`cvtsd2ss`, `sqrtss`,
  `roundss`): they pass `s` as the merge source, which is right only when `s` is an
  XMM register.
- With AVX-512F: `u32`/`u64 -> f32/f64` as `vcvtusi2ss/sd` (the `u64` halving sequence
  goes away too), with the load folded.
- `combine.ax`: `x / c -> x * (1/c)` when `c` is a power of two (and finite, non-zero, and
  its reciprocal is normal), for scalars and vectors.

## Impact

Measured 3-9% on `contact_inputs`/`race_inputs`/`contact_single`. With the
multiplication, estimate 20-40 ns summed over the kernels.

## Resolution

Fixed in 68528df as proposed, with `vzero` rather than a function-wide zero register:

- `cvt_si2f` zeroes its destination first (`vpxor d,d,d`, a zeroing idiom that reads nothing)
  instead of treating it as read, so the register allocator no longer keeps it live (or
  spills it) into the conversion. `sse_unary` does the same when its source is in memory
  (with a register source it already merged into that register).
- With AVX-512F, `u32`/`u64` to `f32`/`f64` is one `vcvtusi2ss`/`vcvtusi2sd` (EVEX.W from
  the operand size, which the EVEX encoder now honours). The load is not folded.
- `combine.ax`: `x / c` becomes `x * (1/c)` when `c` (or every lane of a constant vector) is
  a power of two whose reciprocal is a normal number: the same real number, rounded the same.

`calculate_race_inputs`' conversion is now `vpxor xmm3,xmm3,xmm3; vcvtusi2ss xmm3,xmm3,r12d;
vmulss`. Measured: race_inputs 216 to 202 ns, the rest within noise; see 0057's table.

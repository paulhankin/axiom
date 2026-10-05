# 0089: C compiler: vector `@int_from_float` to `u32` is five instructions on AVX-512 where two do

- Type: feature (performance)
- Status: fixed (4c1d5fc)
- Area: compiler (C)

## Today

Issue 0030 lowered vector `@int_from_float` (spec §13.2: truncate, saturate,
NaN to 0) as a clamp and a plain conversion. For `u32x16` from `f32x16` with
AVX-512 that is (`codegen.c`, `vec_int_from_float`):

```
vcmpleps   k1, two32, x      ; x >= 2^32: saturate to UINT_MAX
vmaxps     t, x, zero        ; NaN and negatives to 0
vminps     t, t, below       ; largest float below 2^32
vcvttps2udq r, t
vmovdqa32  r{k1}, ones
```

AVX-512's `vcvttps2udq` already returns `0xFFFFFFFF` for every input that does
not fit, which is the saturated value for `x >= 2^32` and wrong only for NaN
and for `x <= -1`. So the whole conversion is

```
vcmpgeps    k1, x, zero      ; ordered: false for NaN and negatives
vcvttps2udq r{k1}{z}, x      ; (-1, 0) truncates to 0 anyway
```

that is, `select(x >= 0, llvm.x86.avx512.mask.cvttps2udq.512(x), 0)`, and
likewise for the 128- and 256-bit forms with AVX-512VL and for `f64` lanes
(`vcvttpd2udq`, `vcvttps2uqq`/`vcvttpd2uqq` with AVX-512DQ). The signed forms
are similar: `vcvttps2dq` gives `0x80000000` (`INT_MIN`) for NaN and out of
range, so a compare for `x >= 2^31` and one for NaN with two masked moves is
the whole fix-up, without the two clamps.

The self-hosted compiler already gets two instructions another way: for the same
source it emits `vmaxps` with the zero as the second source (NaN and negatives
to 0) and `vcvttps2udq`, which saturates the rest by itself.

## Measured

the benchmark's AVX-512 sigmoid (the benchmark 86b3494, `axiom/src/eval/sigmoid.ax`,
`sigmoid16`, Rust's `_mm512_sigmoid_ps`) converts its table index with
`@int_from_float(u32x16, x1)`; the contact net runs 17 per pair of positions,
the pruning board kernels about two per board. The same port built by the C
compiler at 37af634 and by that compiler with the two-instruction lowering
above (a local experiment, `u32x16` only), against the Rust engine
(the benchmark `bench_kernels`, ns per position, best of 8 alternating runs on a
4-core AVX-512 Xeon shared with other jobs; all three give the same checksums):

| Kernel | today | `vcmpgeps` + `vcvttps2udq{z}` | Rust |
|---|---|---|---|
| contact net | 378 | 372 | 364 |
| race net | 281 | 274 | 266 |
| pruned race | 195 | 192 | 188 |
| pruned, from boards | 60.9 | 58.2 | 54.3 |

That is 2–4% of the net kernels, about half of what separates the LLVM build
of the port from Rust on them.

## Also seen: `@swizzle2` of `f32` lanes is an integer permute

`swizzle2_lanes` bitcasts float tables to `i32` and calls
`llvm.x86.avx512.vpermi2var.d.512`, so the sigmoid's table lookups are
`vpermt2d`/`vpermi2d` and its `@select`s `vmovdqa32` blends, in the integer
domain between floating-point operations (Rust: `vpermt2ps`, `vmovaps`). Using
`vpermi2var.ps.512` (and `.pd.512` for `f64`) for float lanes gives the float
forms; on this machine that made no measurable difference, so it is noted here
rather than filed on its own.

## Proposal

In `vec_int_from_float`, when the function has AVX-512F (and VL for the narrower
vectors, DQ for 64-bit integer lanes), lower the unsigned conversions to the
masked AVX-512 conversion with a zeroing mask from an ordered `x >= 0`, and the
signed ones to the plain conversion with the two fix-ups. `tests/run/simd_lanes.ax`
already compares every lane with the scalar conversion on the edge values
(±0, -0.5, -1, 2^31, 2^32, ±inf, NaN); an `--emit=asm` check could count the
instructions, as for 0030.

## Fixed

4c1d5fc (`codegen.c`, `x86_cvtt`): where the function's extensions have a
whole-vector conversion, `vec_int_from_float` uses it, called unmasked
(`llvm.x86.avx512.mask.cvtt{ps,pd}2{u,}{dq,qq}.{128,256,512}`, or SSE2's and
AVX's signed `cvttps2dq` / `cvttpd2dq`):

- unsigned: `select(x >= 0 (ordered), cvt(x), 0)`, which LLVM makes the
  zero-masked conversion: `vcmpleps k1, zero, x; vcvttps2udq r{k1}{z}, x`;
- signed: the conversion, then `x >= 2^31` (or 2^63) to the maximum and NaN
  to 0: `vcmpordps`, `vcmpgeps`, `vcvttps2dq` and two masked moves.

Other widths (8- and 16-bit lanes, vectors wider than the registers, half a
register of results) keep the clamps. `@swizzle` / `@swizzle2` of float lanes
use the floating-point permutes (`vpermps`, `vpermt2ps`, `vpermt2pd`); as noted
above that made no measurable difference.

Tests: `tests/lowering/x64_int_from_float.ax` (no `vminps`, one conversion
each; the self-hosted compiler passes it too), `tests/run/simd_lanes.ax`
gains `conversion_widths` (every width x86 converts in one instruction
against the scalar conversions; checked at `--cpu=x86-64`, `-v3`, `-v4`,
`-O0` and `-O2`, with both compilers), and `tests/lowering/avx512_whole.ax`
counts zeroing masks on moves only (the conversion has one now).

Measured (the benchmark bc2f24b `bench_kernels`, ns a position, min of 9
alternating runs, the machine shared, ±3%): base = the C compiler at bada176,
fixed = with this and 0091.

| kernel | base | fixed | Rust |
|---|---|---|---|
| contact net | 382.3 | 372.4 | 369.7 |
| race net | 284.4 | 275.7 | 272.9 |
| pruned boards | 61.0 | 58.6 | 54.2 |
| contact single | 409.9 | 401.1 | 439.9 |

The checksums are unchanged and the benchmark's `bench_search` results stay
bit-identical to Rust's in every mode.

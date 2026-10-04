# 0030: Vector @int_from_float is scalarized on x86

- Type: bug (performance)
- Status: open
- Area: compiler (C)

## Today

`@int_from_float(I, v)` on a vector (issue 0026) is emitted as
`llvm.fptoui.sat` / `llvm.fptosi.sat` on the whole vector
(`codegen.c`, `BI_INT_FROM_FLOAT`). LLVM 18 has no x86 lowering for the
vector forms of these intrinsics and splits them into one scalar conversion
per lane, each with its own range fix-ups. For `u32x16` from `f32x16` that is,
per lane: `vmovss`, `vcvttss2si`, two `vucomiss`, two `cmov`, and a `vpinsrd`
to put the lane back, about 110 instructions where AVX-512 has one
(`vcvttps2udq`). The signed form is the same shape (`vcvttss2si`, `cmova`,
`cmovp`). `@float_from_int` is fine (`vcvtudq2ps`).

```axiom
export fn cu(x: *f32x16, o: *mut u32x16) void { o.* = @int_from_float(u32x16, x.*); }
```

## Measured

the benchmark's AVX-512 kernels (the benchmark `axiom/src/eval/simd_avx512.ax`) take the
table index of every sigmoid as `@int_from_float(u32x16, x1)`, as Rust's
`_mm512_cvttps_epi32`; the contact net evaluates 17 such sigmoids per pair of
positions. Below, the same program built by the C compiler at 79677a0 and by
the same compiler with the lowering proposed here (unsigned case only, as a
local experiment), against the Rust engine (ns per position, best of 6
alternating runs, AVX-512 Xeon; all three give the same checksums):

| Kernel | today | clamp + `fptoui` | Rust |
|---|---|---|---|
| contact net | 515 | 252 | 211 |
| race net | 428 | 186 | 155 |
| pruned contact | 137 | 100 | 81 |
| pruned race | 191 | 126 | 110 |
| contact, one position | 529 | 307 | 260 |
| pruned, one position | 184 | 140 | 108 |
| pruned, from boards | 88 | 54 | 46 |

So the conversion alone makes the port twice Rust's time on the big nets, and
with it fixed the kernels are within 1.1–1.3x.

## Proposal

For vector operands, build the saturating conversion from operations LLVM
lowers well, keeping the §13.2 semantics (truncate; out of range saturates;
NaN gives 0):

- unsigned: `c = select(x > 0, x, 0)` (ordered compare, so NaN and negatives
  give 0), `c = select(c < MAXF, c, MAXF)` with `MAXF` the largest float below
  `2^bits`, then a plain `fptoui`. On x86 that is `vmaxps`, `vminps`,
  `vcvttps2udq` (AVX-512) or the usual `vcvttps2dq` sequence (AVX2);
- signed: `r = fptosi(clamp(x, -2^(bits-1), largest float below 2^(bits-1)))`,
  then `select(x >= 2^(bits-1), MAX, r)` and `select(x != x, 0, r)`:
  `vcvttps2dq` plus two compares and blends.

The experiment above was the unsigned rule (a dozen lines in
`BI_INT_FROM_FLOAT`). On wasm32, keep the intrinsic: LLVM lowers
`fptosi.sat`/`fptoui.sat` on `<4 x float>` to `i32x4.trunc_sat_f32x4_s/u`
directly. A test could check the instruction count of a 16-lane conversion
in `--emit=asm`, as for the other lowering checks.

# 0024: Runtime permutes of 16-, 32- and 64-bit lanes (and fast wide byte swizzles)

- Type: feature
- Status: fixed (107a199)
- Area: language, compiler (C and selfhost)

## Today

`@swizzle(a, idx)` takes byte vectors only. There is no way to pick lanes of
a `f32x16` (or `u32x8`, ...) by runtime indices, which x86 does in one
instruction (`vpermps`/`vpermd`, AVX2; `vpermt2ps`/`vpermt2d` across two
registers, AVX-512) and Arm with `tbl` on bytes.

The common use is a small table kept in registers. the benchmark's sigmoid (and
Rust's `_mm512_sigmoid_ps`) looks up 16 lanes in a 128-entry `f32` table
with 4 `vpermt2ps` and 3 blends. In Axiom the 16 entries have to be loaded
one at a time (`@vec(f32x16, T[@extract_lane(i, 0)], ...)`), which LLVM
compiles to 16 lane extracts, 16 loads and 15 inserts.

Wide byte swizzles have a lowering problem too: the C compiler splits a
`u8x32`/`u8x64` `@swizzle` into one `pshufb` per pair of 16-byte pieces and
rebuilds the result with an `insertelement` per byte, even where the
processor has `vpermb` (AVX-512VBMI) or can use `vpshufb` on 256 bits.

## Measured

16-lane sigmoid on 4,096 values (native, AVX-512, ns per 16 lanes):

| | ns |
|---|---|
| Rust, `vpermt2ps` table in registers | 3.95 |
| Axiom, lane-by-lane loads (`@vec` of 16 lookups) | 12.9 |
| Axiom, indices stored, entries loaded in a loop | 13.0 |

the benchmark's contact net evaluates 17 such sigmoids per pair of positions, so
this alone is about 75 ns of the 130 ns per position between the Axiom port
(303 ns) and Rust (199 ns), with identical results.

## Proposal

1. `@swizzle(a, idx)` for any lane type: `a` a vector of N lanes, `idx` an
   unsigned integer vector of N lanes of the same lane width; lane i of the
   result is `a[idx[i]]`, or zero when `idx[i] >= N` (today's rule for
   bytes).
2. `@swizzle2(a, b, idx)`: the same over the 2N lanes of `a` then `b`
   (`vpermt2*`), so a 64-entry `f32` table is two registers and one
   instruction, and a 128-entry one is four and three `@select`s.
3. Lowering:
   - AVX-512: `vpermps/d/q`, `vpermt2ps/d/q`, `vpermw`/`vpermt2w` (BW),
     `vpermb`/`vpermt2b` (VBMI), with a compare into a zeroing mask for
     out-of-range indices (none when the index vector is masked by the
     program, e.g. `idx & 15`: worth recognising).
   - AVX2: `vpermps`/`vpermd` for 8 lanes, `vpshufb` per 128-bit half plus
     a blend for wider tables.
   - wasm32/SSE/NEON: byte indices `lanebytes * idx + k` and `i8x16.swizzle`/
     `pshufb`/`tbl` per 16-byte table piece, OR-ed together (out-of-range
     pieces give zero).
4. Independently: lower `u8x32`/`u8x64` `@swizzle` to `vpshufb ymm`+blend or
   `vpermb`/`vpermt2b` where available, without per-byte inserts.

## Not chosen

- An `@x86_permutex2var` builtin, like the `@x86_*` crypto ones: simpler,
  but code using it needs a second, slow path for every other target.
- `@gather(slice, idx)`: a memory gather (`vgatherdps`) is slower than a
  register permute for tables that fit in registers, and needs bounds
  checks or masking per lane.

## Resolution

`@swizzle(a, idx)` takes a vector of any lanes, with indices of the same shape (an integer vector of the lanes' width, taken as unsigned; out of range gives 0), and `@swizzle2(a, b, idx)` looks up the 2n lanes of `a` then `b` (spec §13.2). The C compiler uses `vpermd/q`, `vpermw` (BW), `vpermb` (VBMI) and `vpermt2*` for 512-bit tables, `vpermd`/`vpermps` for 8 lanes with AVX2, with a compare to clear out-of-range lanes (which LLVM drops when the indices are masked, e.g. `idx & @splat(31)` gives one `vpermt2d`); elsewhere the indices become byte indices into a byte `@swizzle`. Wide byte swizzles join their 16-byte pieces with shuffles instead of an insert per byte, and `u8x64` uses `vpermb` where the processor has VBMI. The self-hosted compiler lowers every lane type to byte swizzles (`i8x16.swizzle`, `pshufb`, `tbl` per piece). tests/run/simd_lanes.ax checks both.

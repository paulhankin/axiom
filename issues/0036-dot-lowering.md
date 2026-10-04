# 0036: `@dot` on `i16` lanes does not become `pmaddwd` / `i32x4.dot_i16x8_s`

- Type: bug (performance)
- Status: fixed (902daad)
- Area: compiler (C)

## Today

`@dot(a, b)` on `i16` lanes (Appendix A: WASM `i32x4.dot_i16x8_s`, half as many
`i32` lanes) is emitted as generic IR: sign-extend, multiply, take the even and
odd lanes and add. LLVM recognises that as `pmaddwd` only sometimes. Where an
operand is a constant or known small, it "simplifies" the multiply first and
loses the pattern. the benchmark's `half_point_sums_avx512` (the benchmark
`axiom/src/eval/features.ax`) takes eight `@dot`s of `i16x32` vectors:

```axiom
@dot(n, one), @dot(n, point), @dot(n, w_break), @dot(n, w_free), @dot(timed, one), ...
```

Three become `vpmaddwd`. The other five become `vpmovzxwd` (twice), `vpermt2d`,
`vpsrld`, `vpaddd` and sometimes `vpmulld` (10 cycles), several instructions
each. Rust's `_mm512_madd_epi16` gives 8 `vpmaddwd`. The function takes about
46 ns per position natively against Rust's 25. On wasm32 none of the port's
`@dot`s is `i32x4.dot_i16x8_s` (8 `extmul`/`extadd_pairwise` sequences in
`bench_kernels`'s module).

## Proposal

Emit the target's instruction directly for `i16` `@dot`:
`llvm.x86.avx512.pmaddw.d.512` / `llvm.x86.avx2.pmadd.wd` /
`llvm.x86.sse2.pmadd.wd` by vector width and features (splitting wider vectors
where the width isn't available), `llvm.wasm.dot` on wasm32, and `smull`/
`sadalp`-style sequences (or the generic IR) elsewhere. The semantics are the
same (the sum of two `i16` products, wrapping only for `-32768 * -32768 * 2`,
which `pmaddwd` and wasm agree on).

## Resolution

The C compiler emits the instruction for `@dot` of `i16` vectors (`dot_i16_native` in `codegen.c`): `llvm.x86.avx512.pmaddw.d.512`, `llvm.x86.avx2.pmadd.wd` or `llvm.x86.sse2.pmadd.wd`, the widest the function's extensions have (so `@features`/`@multiversion` versions get theirs), splitting wider vectors into registers of that width and joining the results; `llvm.wasm.dot` per 128 bits on wasm with `simd128`; the generic IR elsewhere (AArch64, and wasm without SIMD). The semantics do not change (Appendix A now states them: lane `i` is `a[2i] * b[2i] + a[2i+1] * b[2i+1]` in `i32`, wrapping only for two `-32768 * -32768` products).

the benchmark's `half_point_sums` shape (eight `@dot`s of `i16x32`, five with constant operands) is now 8 `vpmaddwd` with AVX-512 (was 5 plus `vpermt2d` and friends), 16 with AVX2, 32 `pmaddwd` with SSE2, and 32 `i32x4.dot_i16x8_s` on wasm (was none: 8 `extmul` sequences). the benchmark's `bench_kernels`, native AVX-512, `-O2`, best of 7 on a loaded machine: contact_inputs 197 -> 174 ns, contact_single 335 -> 285 ns, with the same checksums.

Tests: `tests/run/dot_i16.ax` compares every lane with the scalar sums on edge values (all `-32768`/`32767` pairs, 128-, 256- and 512-bit vectors at every offset, constant operands), on wasm and native at every level. `tests/lowering/` is a new kind of test (`tests/run.mjs`): each `// lower: <flags> | <regex> | <count>` line builds the file with `--emit=asm` and counts the matches, so instruction selection is checked; `tests/lowering/dot_i16.ax` has the counts above (x86 lines run on x86-64 hosts; a compiler without `--emit=asm`, the self-hosted one, skips the file).

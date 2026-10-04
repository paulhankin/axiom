# 0036: `@dot` on `i16` lanes does not become `pmaddwd` / `i32x4.dot_i16x8_s`

- Type: bug (performance)
- Status: open
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
